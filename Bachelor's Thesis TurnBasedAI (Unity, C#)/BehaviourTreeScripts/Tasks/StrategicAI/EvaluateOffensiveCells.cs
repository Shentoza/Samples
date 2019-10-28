using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


/// <summary>
/// Evaluates Cells that are valuable for an attack
/// Priority = (ChanceToHit*weightHitChance + tension*weightTension)²
/// </summary>
public class EvaluateOffensiveCells : ITask
{
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;


    private float weightHitChance;
    private float weightTension;

    public EvaluateOffensiveCells(IParentNode parent, MyBehaviour behaviour,float weightHitChance = 3.0f, float weightInfluence = 1.0f) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.weightHitChance = weightHitChance;
        this.weightTension = weightInfluence;
    }

    public void activate()
    {
        //Debug.Log("Find attack cells");
        isActive = true;
        behaviour.activateTask(this);
    }

    public void deactivate()
    {
        if (isActive) {
            //Debug.Log("Find attack cells Done");
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    /// <summary>
    /// Calculates safe cells based on hitchance for the enemyUnits
    /// Priority = HitChance
    /// </summary>
    public void run(double deltaTime) {

        PlayerComponent opponent = null;
        foreach (PlayerComponent p in GameObject.FindObjectsOfType<PlayerComponent>())
        {
            if (null != p.gameObject.GetComponent<inputSystem>())
            {
                opponent = p;
            }
        }

        Dictionary<Cell, float[]> Cell_ProtectionDict = new Dictionary<Cell, float[]>();
        List<ArrayList> result = new List<ArrayList>();
        ShootingSystem shoot = ShootingSystem.Instance;

        float DEBUG_maxHitChance = Mathf.NegativeInfinity;
        Cell maxCell = null;

        foreach (GameObject target in opponent.figurines) {
            Cell_ProtectionDict.Clear();
            maxCell = null;
            DEBUG_maxHitChance = Mathf.NegativeInfinity;
            foreach(GameObject g in BattlefieldCreater.instance.getZellen())
            {
            
                Cell currentCell = g.GetComponent<Cell>();
                if (currentCell.hoheDeckung || currentCell.niedrigeDeckung || currentCell.isOccupied)
                    continue;
                int x = currentCell.xCoord;
                int z = currentCell.zCoord;

                float hitChance = shoot.hitChanceBetweenCells(target.GetComponent<AttributeComponent>().getCurrentCell(), currentCell);
                if (hitChance == -1.0f)
                    continue;


                Cell_ProtectionDict.Add(currentCell, new float[] { hitChance, hitChance, 0.0f}) ;

                if(hitChance > DEBUG_maxHitChance) {
                    DEBUG_maxHitChance = hitChance;
                    maxCell = currentCell;
                }
            }
            if (maxCell != null)
                Debug.Log("EVAL DEBUG CALC: Target: " + target.name + ": Cell" + maxCell.name + "   HitChance:" + DEBUG_maxHitChance);
            
            foreach(Cell currentCell in Cell_ProtectionDict.Keys) {
                ArrayList subResult = new ArrayList();
                subResult.Add(Cell_ProtectionDict[currentCell][0]); //Priority
                subResult.Add(currentCell); //Cell
                subResult.Add(Cell_ProtectionDict[currentCell][1]); //Hitchance
                subResult.Add(Cell_ProtectionDict[currentCell][2]); //Tension (0.0f)
                subResult.Add(target);

                Utility.SortDataByPriority(subResult, result);
            }

            Debug.Log(Enums.BlackboardNotes.OFFENSIVE_CELL + " NOTES " + result.Count);
            foreach (ArrayList data in result)
                AIBlackboard.Instance.WriteToBlackboard(new OffensiveCellNote(data));
            result.Clear();
        }

        parent.finishChild(this, true);
        deactivate();
    }

    public void ResetNode() {
        deactivate();
    }
}
