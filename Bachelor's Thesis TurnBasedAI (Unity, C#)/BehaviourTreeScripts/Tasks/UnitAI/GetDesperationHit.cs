using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GetDesperationHit :ITask {
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;
    bool isInitialized;

    GameObject unit;
    PlayerComponent opponent;

    public GetDesperationHit(IParentNode parent, MyBehaviour behaviour, GameObject unit, PlayerComponent opponent) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.unit = unit;
        this.opponent = opponent;
    }

    public void activate() {
        Debug.Log("DesperationHit Possible?");
        isActive = true;
        behaviour.activateTask(this);
    }

    public void deactivate() {
        if (isActive) {
            Debug.Log("DesperationHit Possible Done");
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    public void run(double deltaTime) {
        ShootingSystem shoot = ShootingSystem.Instance;
        float highestChance = 0.0f;
        GameObject target = null;
        AttributeComponent myAttributes = unit.GetComponent<AttributeComponent>();

        foreach(GameObject enemy in opponent.figurines) {
			if(!enemy)
					continue;
			
            float hitChance = shoot.hitChanceBetweenCells(enemy.GetComponent<AttributeComponent>().getCurrentCell(), myAttributes.getCurrentCell());
            if(hitChance > highestChance) {
                highestChance = hitChance;
                target = enemy;
            }
        }


        unit.GetComponent<UnitBlackboard>().targetUnit = target;
        if (target == null)
            parent.finishChild(this, false);
        else
            parent.finishChild(this, true);

        deactivate();
    }

    public void ResetNode() {
        deactivate();
    }
}
