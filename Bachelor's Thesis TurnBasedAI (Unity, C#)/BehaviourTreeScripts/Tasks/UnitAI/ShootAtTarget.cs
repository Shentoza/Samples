using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Gets the best offensive Cell from blackboard, moves there, and tries to shoot the enemy
/// </summary>
public class ShootAtTarget : ITask
{
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;
    bool isInitialized;

    PlayerComponent player;
    GameObject unit;
    bool isDone;

    public ShootAtTarget(IParentNode parent, MyBehaviour behaviour, GameObject unit, PlayerComponent player) { 
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.unit = unit;
        this.player = player;
    }

    public void activate()
    {
        Debug.Log("ShootFromBestPosition");
        isActive = true;
        behaviour.activateTask(this);

        isInitialized = false;
        isDone = false;
    }

    public void deactivate()
    {
        if(isActive) {
            Debug.Log("ShootFromBestPosition Done");
            isActive = false;
            behaviour.deactivateTask(this);

            isInitialized = false;
        }
    }

    public void run(double deltaTime) {
        UnitBlackboard blackboard = unit.GetComponent<UnitBlackboard>();
        AttributeComponent myAttributes = unit.GetComponent<AttributeComponent>();

        if(!isInitialized) {
            if(ShootingSystem.Instance.hitChanceBetweenCells(blackboard.targetUnit.GetComponent<AttributeComponent>().getCurrentCell(),unit.GetComponent<AttributeComponent>().getCurrentCell()) != -1.0f) {
                ShootingSystem.Instance.StartCoroutine(ShootingSystem.Instance.makeShot(unit, blackboard.targetUnit, player));
                isInitialized = true;
            }
            else {
                parent.finishChild(this, false);
                deactivate();
            }
        }

        if (myAttributes.shooting)
            return;
        else if(!isDone && isInitialized) {
            isDone = true;
            parent.finishChild(this, true);
            deactivate();

        }

    }

    public void ResetNode() {
        deactivate();
    }
}
