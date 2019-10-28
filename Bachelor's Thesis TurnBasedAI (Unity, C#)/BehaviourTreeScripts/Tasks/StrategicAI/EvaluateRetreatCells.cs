using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


/// <summary>
/// Calculates retreat cells based on a low tension and the chance of getting hit there
/// Priority = ((1-ChanceGettingHit)*WeightHitChance + (Tension/MaxTension)*WeightTension)²
/// </summary>
public class EvaluateRetreatCells : ITask
{
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;

    private float weightHitChance;
    private float weightInfluence;

    public EvaluateRetreatCells(IParentNode parent, MyBehaviour behaviour, float weightHitChance = 2.0f, float weightInfluence = 1.0f) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.weightHitChance = weightHitChance;
        this.weightInfluence = weightInfluence;
    }

    public void activate()
    {
        //Debug.Log("ITASK: Find retreat nodes");
        isActive = true;
        behaviour.activateTask(this);
    }

    public void deactivate()
    {
        if(isActive) {
            //Debug.Log("ITASK: Find retreat nodes Done");
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
        InfluenceMap.Instance.calculateTeamInfluence(1);
        InfluenceMap.Instance.calculateTeamInfluence(2);
        float[,] tensionMap = InfluenceMap.Instance.generateTensionMap();
        float maxTension = InfluenceMap.Instance.maxTension;

        foreach(GameObject g in zellen)
        {
            Cell currentCell = g.GetComponent<Cell>();
            if (currentCell.isOccupied)
                continue;
            int x = currentCell.xCoord;
            int z = currentCell.zCoord;
            float tension = tensionMap[x, z];

            foreach(GameObject enemyUnit in opponent.figurines)
            {
				if(!enemyUnit)
					continue;
				
                float hitChance = shoot.hitChanceBetweenCells(currentCell, enemyUnit.GetComponent<AttributeComponent>().getCurrentCell());
                if (hitChance == -1.0f)
                    hitChance = 0.0f;

                // Priorität der Zelle: 1-Hitchance*GewichtungHitchance + Einfluss(0..1)*GewichtungEinfluss
                float weightedInfluence = (1-(tensionMap[x, z] / maxTension)) * weightInfluence;
                float weightedhitChance = (1 - hitChance) * weightHitChance;
                float priority = weightedhitChance * weightedInfluence;

                // Noch nicht im Dictionary? Eintrag anlegen
                if (!Cell_ProtectionDict.ContainsKey(currentCell)) {
                    Cell_ProtectionDict.Add(currentCell, new float[] { priority, hitChance, tension});
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
            subResult.Add(Cell_ProtectionDict[currentCell][2]); //Tension

            Utility.SortDataByPriority(subResult, result);
        }

        //Debug.Log(Enums.BlackboardNotes.RETREAT_CELL + " NOTES " + result.Count);
        foreach (ArrayList data in result)
            AIBlackboard.Instance.WriteToBlackboard(new RetreatCellNote(data));

        parent.finishChild(this, true);
        deactivate();
    }

    public void ResetNode() {
        deactivate();
    }
}
