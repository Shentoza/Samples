using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AreThereEnemies : ITask {

    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;

    public AreThereEnemies(IParentNode parent, MyBehaviour behaviour) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;
    }

    public void activate() {
        //Debug.Log("Evaluate: Are there enemies?");
        behaviour.activateTask(this);
        isActive = true;
    }

    public void deactivate() {
        if(isActive) {
            behaviour.deactivateTask(this);
            isActive = false;
        }
    }
    public void run(double deltaTime) {
        if (!isActive)
            return;

        PlayerComponent opponent = null;
        foreach(PlayerComponent p in GameObject.FindObjectsOfType<PlayerComponent>()) {
            if(null != p.gameObject.GetComponent<inputSystem>()) {
                opponent = p;
            }
        }
        if (opponent != null) {
            foreach(GameObject g in opponent.figurines)
            { 
                if(g != null)
                {
                    deactivate();
                    parent.finishChild(this, true);
                    return;
                }
            }
            parent.finishChild(this, false);
            deactivate();
        }
        else
            Debug.Log("No opponent found!");

    }

    public void ResetNode() {
        deactivate();
    }
}
