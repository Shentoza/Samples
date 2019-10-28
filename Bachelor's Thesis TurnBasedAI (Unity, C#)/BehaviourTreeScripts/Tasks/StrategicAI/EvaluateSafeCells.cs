using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Calculates safe cells based on the influence values for a certain team, and the chances of getting hit there
/// Priority = ((1-ChanceGettingHit)*WeightHitChance + (Influence/maxInfluence)*WeightHitChance)²
/// </summary>
public class EvaluateSafeCells : ITask
{
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;

    private float weightHitChance;
    private float weightInfluence;

    public EvaluateSafeCells(IParentNode parent, MyBehaviour behaviour, float weightHitChance = 1.0f, float weightInfluence = 1.0f) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.weightHitChance = weightHitChance;
        this.weightInfluence = weightInfluence;
    }

    public void activate()
    {
        //Debug.Log("ITASK: Find safe nodes");
        isActive = true;
        behaviour.activateTask(this);

        InfluenceMap.Instance.calculateTeamInfluence(1);
        InfluenceMap.Instance.calculateTeamInfluence(2);
    }

    public void deactivate()
    {
        if(isActive) {
            //Debug.Log("ITASK: Find safe nodes Done");
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    public void run(double deltaTime) {

        PlayerComponent opponent = null;
        foreach (PlayerComponent p in GameObject.FindObjectsOfType<PlayerComponent>())
        {
            if (null != p.gameObject.GetComponent<inputSystem>())
            {
                opponent = p;
            }
        }
        GameObject[,] zellen = BattlefieldCreater.instance.getZellen();
        Dictionary<Cell, float[]> Cell_ProtectionDict = new Dictionary<Cell, float[]>();
        ShootingSystem shoot = ShootingSystem.Instance;
        

        float[,] influenceMap = InfluenceMap.Instance.generateInfluenceMap(2);
        float maxInfluence = InfluenceMap.Instance.maxInfluence;

        foreach(GameObject g in zellen)
        {
            Cell currentCell = g.GetComponent<Cell>();
            if (currentCell.isOccupied)
                continue;
            int x = currentCell.xCoord;
            int z = currentCell.zCoord;
            float influence = influenceMap[x, z];

            foreach(GameObject enemyUnit in opponent.figurines)
            {
				if(!enemyUnit)
					continue;
				
                Cell enemyCell = enemyUnit.GetComponent<AttributeComponent>().getCurrentCell();
                float hitChance = shoot.hitChanceBetweenCells(currentCell, enemyUnit.GetComponent<AttributeComponent>().getCurrentCell());
                if (hitChance == -1.0f)
                    hitChance = 0.0f;

                float weightedInfluence = (influenceMap[x, z] / maxInfluence) * weightInfluence;
                float weightedhitChance = (1 - hitChance) * weightHitChance;
                float priority = weightedhitChance + weightedInfluence;

                // Noch nicht im Dictionary? Eintrag anlegen
                if (!Cell_ProtectionDict.ContainsKey(currentCell)) {
                    Cell_ProtectionDict.Add(currentCell, new float[] { priority, hitChance, influence}) ;
                }
                // Erreicht die Zelle ansonsten einen schlechteren Wert?
                // endgültiger Wert = Angreifer mit der höchsten Wahrscheinlichkeit zu treffen
                else {
                    if(priority < Cell_ProtectionDict[currentCell][0]) {
                        Cell_ProtectionDict[currentCell][0] = priority;
                        Cell_ProtectionDict[currentCell][1] = hitChance;
                    }
                }
            }
        }
        List<ArrayList> result = new List<ArrayList>();
        foreach(Cell currentCell in Cell_ProtectionDict.Keys) {
            ArrayList subResult = new ArrayList();
            subResult.Add(Cell_ProtectionDict[currentCell][0]); //Priority
            subResult.Add(currentCell); //Cell
            subResult.Add(Cell_ProtectionDict[currentCell][1]); //HitChance
            subResult.Add(Cell_ProtectionDict[currentCell][2]); //Influence

            Utility.SortDataByPriority(subResult, result);
        }

        //Debug.Log(Enums.BlackboardNotes.SAFEST_CELL + " NOTES " + result.Count);
        foreach (ArrayList data in result)
            AIBlackboard.Instance.WriteToBlackboard(new SafestCellNote(data));

        parent.finishChild(this, true);
        deactivate();
    }

    public void ResetNode() {
        deactivate();
    }
}
