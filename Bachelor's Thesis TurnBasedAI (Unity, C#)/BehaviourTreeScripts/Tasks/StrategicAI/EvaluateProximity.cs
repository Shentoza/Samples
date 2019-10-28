using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public class EvaluateProximity : ITask
{
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;

    public EvaluateProximity(IParentNode parent, MyBehaviour behaviour) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;
    }

    public void activate()
    {
        Debug.Log("Find closest Enemy activated");
        isActive = true;
        behaviour.activateTask(this);
    }

    public void deactivate()
    {
        if (isActive) {
            Debug.Log("Find closest Enemy deactivated");
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    public void run(double deltaTime) {
        PlayerComponent opponent = null;
        PlayerComponent ownTeam = null;
        foreach (PlayerComponent p in GameObject.FindObjectsOfType<PlayerComponent>()) {
            if (null != p.gameObject.GetComponent<inputSystem>()) {
                opponent = p;
            }
            else {
                ownTeam = p;
            }
        }
        AttributeComponent opposingUnitAttr, ownUnitAttr;
        int minDistance = int.MaxValue;


        //Lediglich Suche nach kürzester Distanz für spätere Priorität
        foreach (GameObject opposingUnit in opponent.figurines) {
            opposingUnitAttr = opposingUnit.GetComponent<AttributeComponent>();
            DijkstraSystem.Instance.executeDijsktra(opposingUnitAttr.getCurrentCell());

            foreach(GameObject ownUnit in ownTeam.figurines) {
				if(!ownUnit)
					continue;
				
                ownUnitAttr = ownUnit.GetComponent<AttributeComponent>();
                if (ownUnitAttr.getCurrentCell().dij_GesamtKosten < minDistance)
                    minDistance = ownUnitAttr.getCurrentCell().dij_GesamtKosten;
            }
        }


        CellSorterAscending sorter = new CellSorterAscending();
        foreach(GameObject opposingUnit in opponent.figurines)
        {
			if(!opposingUnit)
				continue;
			
            opposingUnitAttr = opposingUnit.GetComponent<AttributeComponent>();
            DijkstraSystem.Instance.executeDijsktra(opposingUnitAttr.getCurrentCell());
            List<GameObject> unitList = new List<GameObject>();

            foreach(GameObject ownUnit in ownTeam.figurines)
			{
				if(!ownUnit)
					continue;
				
				unitList.Add(ownUnit);
			}
                
            unitList.Sort(sorter);

            //Baue Blackboard Note auf
            ArrayList finalList = new ArrayList();
            float priority = (float)minDistance / (float)unitList[0].GetComponent<AttributeComponent>().getCurrentCell().dij_GesamtKosten;
            Debug.Log("closest Unit for " + opposingUnit.name + ":" + unitList[0].name+ "  | Priority: "+priority);

            foreach (GameObject g in unitList)
                finalList.Add(g);

            CloseEnemyNote note = new CloseEnemyNote(opposingUnit, finalList, priority);
            AIBlackboard.Instance.WriteToBlackboard(note);
        }
        parent.finishChild(this, true);
        deactivate();
    }

    public void ResetNode() {
        deactivate();
    }
}
