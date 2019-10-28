using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Gets the best offensive Cell from blackboard, moves there, and tries to shoot the enemy
/// </summary>
public class MoveToCell : ITask
{
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;
    bool isInitialized;
    bool isDone;

    GameObject unit;
    AttributeComponent unitAttributes;
    Enums.BlackboardNotes targetCellType;
    PlayerComponent player;


    public MoveToCell(IParentNode parent, MyBehaviour behaviour, GameObject unit, PlayerComponent player, Enums.BlackboardNotes typeOfCell) { 
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.unit = unit;
        this.player = player;
        unitAttributes = unit.GetComponent<AttributeComponent>();
        targetCellType = typeOfCell;
    }

    public void activate()
    {
        Debug.Log("ITASK: MoveToCell "+targetCellType);
        isActive = true;
        behaviour.activateTask(this);

        isInitialized = false;
        isDone = false;
    }

    public void deactivate()
    {
        if(isActive) {
            Debug.Log("ITASK: MoveToCell "+targetCellType+"   Done");
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    public void run(double deltaTime) {
        UnitBlackboard blackboard = unit.GetComponent<UnitBlackboard>();
        IBlackboardNote note = blackboard.getNote(targetCellType);
        if(note == null) {
            parent.finishChild(this, false);
            deactivate();
            return;
        }

        Cell targetCell = (Cell) note.getData()[0];
        if (!isInitialized) {
            Debug.Log("EVAL DEBUG INIT: NoteType" + targetCellType);
            DijkstraSystem.Instance.executeDijsktra(unitAttributes.getCurrentCell());
            Cell adjustedTarget = targetCell;

            int possibleAP = blackboard.assignedAP;
            if (targetCellType == Enums.BlackboardNotes.OFFENSIVE_CELL) {
                adjustedTarget = targetCell = blackboard.adjustedShotCell;
                if(unitAttributes.actMovRange + unitAttributes.regPerAP * blackboard.assignedAP < blackboard.adjustedShotCell.dij_GesamtKosten) {
                    parent.finishChild(this, false);
                    deactivate();
                    return;
                }
                //possibleAP = blackboard.offensiveAP;
            }


            if (unitAttributes.actMovRange == 0) {
                if(possibleAP > 0 && AbilitySystem.Instance.RegenerateMovementPoints(player, unit)) {
                    blackboard.assignedAP--;
                    //if (targetCellType == Enums.BlackboardNotes.OFFENSIVE_CELL)
                    //    blackboard.offensiveAP--;
                }
                else {
                    parent.finishChild(this, false);
                    deactivate();
                    return;
                }
            }
            
            while (adjustedTarget.dij_GesamtKosten > unitAttributes.actMovRange) {
                if(adjustedTarget.isOccupied) {
                    adjustedTarget = Utility.lookForNextPossibleNeighbour(adjustedTarget);
                }
                else {
                    adjustedTarget = adjustedTarget.dij_Vorgaenger;
                }
            }
            Debug.Log("EVAL DEBUG:   " + targetCell.name + "(" + targetCell.dij_GesamtKosten + ")   => " + adjustedTarget + "(" + adjustedTarget.dij_Vorgaenger.dij_GesamtKosten + ")");

            MovementSystem.Instance.StartCoroutine(MovementSystem.Instance.MoveTo(adjustedTarget, unitAttributes));
            isInitialized = true;
        }
        if (unitAttributes.moving) {
            return;
        }
        else if(!isDone) {
            isDone = true;
            if (unitAttributes.getCurrentCell() == targetCell) {
                Debug.Log("EVAL DEBUG " + targetCellType + " reached Dest");
                parent.finishChild(this, true);
                deactivate();
            }
            else {
                Debug.Log("EVAL DEBUG " + targetCellType + " didnt reach dest");
                parent.finishChild(this, false);
                deactivate();
            }
        }
    }

    public void ResetNode() {
        deactivate();
    }
}
