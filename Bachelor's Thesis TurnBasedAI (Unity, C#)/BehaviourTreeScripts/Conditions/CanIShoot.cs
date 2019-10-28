using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CanIShoot :ITask {
    string name;
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;

    GameObject unit;

    public CanIShoot(IParentNode parent, MyBehaviour behaviour, GameObject unit, string name = null) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;

        this.unit = unit;
    }

    public void activate() {
        if(!string.IsNullOrEmpty(name))
            Debug.Log("Evaluate: Can i still shoot?  for " + unit.name);

        AttributeComponent attributes = unit.GetComponent<AttributeComponent>();
        if (attributes.canShoot) {
            if (!string.IsNullOrEmpty(name))
                Debug.Log("I can shoot! " + unit.name);
            parent.finishChild(this, true);
        }
        else {
            if (!string.IsNullOrEmpty(name))
                Debug.Log("I canTTT shoot! " + unit.name);
            parent.finishChild(this, false);
        }

        deactivate();
    }

    public void deactivate() {
        if (isActive) {
            isActive = false;
        }
    }
    public void run(double deltaTime) {
        if (!isActive)
            return;
    }

    public void ResetNode() {
		deactivate();
    }
}
