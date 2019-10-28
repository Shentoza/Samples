using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FillUnitBlackboard :ITask {
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;

    GameObject unit;
    AttributeComponent unitAttributes;

    public FillUnitBlackboard(IParentNode parent, MyBehaviour behaviour, GameObject unit, PlayerComponent player) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.unit = unit;
        unitAttributes = unit.GetComponent<AttributeComponent>();
    }

    public void activate() {
        Debug.Log("FillUnitBlackBoard " + unit.name);
        isActive = true;
        behaviour.activateTask(this);
        DijkstraSystem.Instance.executeDijsktra(unitAttributes.getCurrentCell());
    }

    public void deactivate() {
        if (isActive) {
            Debug.Log("FillUnitBlackboard Done " + unit.name);
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    public void run(double deltaTime) {
        UnitBlackboard blackboard = unit.GetComponent<UnitBlackboard>();
        AttributeComponent targetAttr = null;

        ArrayList notes = AIBlackboard.Instance.GetAllNotesOfType(Enums.BlackboardNotes.WEAKEST_ENEMY);
        foreach(IBlackboardNote tmpNote in notes) {
            GameObject target = (GameObject)tmpNote.getData()[0];
            targetAttr = target.GetComponent<AttributeComponent>();
            if (target != null && targetAttr.hp > 0.0f) {
                blackboard.targetUnit = target;
                break;
            }
        }
        notes.Clear();


        int weaponRange = unit.GetComponent<InventoryComponent>().getCurrentWeapon().weaponRange;
        float maxPriority = -100.0f;
        int minRange = 10000;
        notes = AIBlackboard.Instance.GetAllNotesOfType(Enums.BlackboardNotes.OFFENSIVE_CELL);
        foreach (IBlackboardNote tmpNote in notes) {
            Cell currentCell = (Cell)tmpNote.getData()[0];
            float priority = tmpNote.getPriority(); ;
            GameObject target = (GameObject)tmpNote.getData()[3];
            targetAttr = target.GetComponent<AttributeComponent>();

            if(target == blackboard.targetUnit) {

                if (Mathf.Abs(target.GetComponent<AttributeComponent>().getCurrentCell().dij_GesamtKosten - currentCell.dij_GesamtKosten) <= weaponRange) {
                    //Bessere Hitchance, oder mindestens gleichgute Hitchance aber kürzerer Weg?
                    if ((priority > maxPriority) ||  (priority >= maxPriority && currentCell.dij_GesamtKosten < minRange) ) {
                        blackboard.adjustedShotCell = currentCell;
                        blackboard.offensivePosition = tmpNote;
                        maxPriority = priority;
                        minRange = currentCell.dij_GesamtKosten;
                        //Debug.Log("EVAL DEBUG UPDATE OFFENSIVE!#########");
                        //Debug.Log("EVAL DEBUG   " + target.name);
                        //Debug.Log("EVAL DEBUG   Hit%: " + hitChance + "   oldPrio:" + maxPriority);
                        //Debug.Log("EVAL DEBUG   Zelle: " + currentCell.name);
                        //Debug.Log("EVAL DEBUG   Kosten: " + currentCell.dij_GesamtKosten);
                    }
                }
            }
        }
        notes.Clear();

        maxPriority = -100.0f;
        minRange = 10000;
        notes = AIBlackboard.Instance.GetAllNotesOfType(Enums.BlackboardNotes.SAFEST_CELL);
        foreach (IBlackboardNote tmpNote in notes) {
            Cell currentCell = (Cell)tmpNote.getData()[0];
            if(!currentCell.isOccupied) {
                //Bessere Priority, oder mindestens gleichgute Priority aber kürzerer Weg?
                if((tmpNote.getPriority() > maxPriority) || (tmpNote.getPriority() >= maxPriority && currentCell.dij_GesamtKosten < minRange)) {
                    blackboard.safePosition = tmpNote;
                    tmpNote.isAssigned = true;
                }
            }
        }
        notes.Clear();


        maxPriority = Mathf.NegativeInfinity;
        minRange = 10000;
        notes = AIBlackboard.Instance.GetAllNotesOfType(Enums.BlackboardNotes.RETREAT_CELL);
        foreach (IBlackboardNote tmpNote in notes) {
            Cell currentCell = (Cell)tmpNote.getData()[0];
            if (!currentCell.isOccupied) {
                if ((tmpNote.getPriority() > maxPriority) || (tmpNote.getPriority() >= maxPriority && currentCell.dij_GesamtKosten < minRange)) {
                    blackboard.retreatPosition = tmpNote;
                    minRange = currentCell.dij_GesamtKosten;
                    maxPriority = tmpNote.getPriority();
                    tmpNote.isAssigned = true;
                }
            }
        }
        notes.Clear();

        parent.finishChild(this, true);
        deactivate();
    }

    public void ResetNode() {
        deactivate();
    }
}
