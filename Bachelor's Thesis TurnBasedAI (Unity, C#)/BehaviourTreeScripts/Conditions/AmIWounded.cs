using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AmIWounded :ITask {

    IParentNode parent;
    MyBehaviour behaviour;
    int minHP;
    GameObject unit;
    bool isActive;

    public AmIWounded(IParentNode parent, MyBehaviour behaviour, GameObject unit, int minHP) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.unit = unit;
        this.minHP = minHP;
    }

    public void activate() {
        behaviour.activateTask(this);
        isActive = true;
    }

    public void deactivate() {
        if (isActive) {
            behaviour.deactivateTask(this);
            isActive = false;
        }
    }
    public void run(double deltaTime) {
        if (!isActive)
            return;

        AttributeComponent attributes = unit.GetComponent<AttributeComponent>();
        if (attributes.hp < minHP) {
            parent.finishChild(this, true);
        }
        else {
            parent.finishChild(this, false);
        }

        deactivate();

    }

    public void ResetNode() {
        deactivate();
    }
}
