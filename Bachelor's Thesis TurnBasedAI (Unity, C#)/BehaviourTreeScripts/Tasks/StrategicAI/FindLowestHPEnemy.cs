using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FindLowestHPEnemy : ITask
{
    IParentNode parent;
    MyBehaviour behaviour;
    bool isActive;

    public FindLowestHPEnemy(IParentNode parent, MyBehaviour behaviour) {
        this.parent = parent;
        this.behaviour = behaviour;
        isActive = false;
    }

    public void activate()
    {
        //Debug.Log("ITASK: Find lowest HP Enemy activated");
        isActive = true;
        behaviour.activateTask(this);
    }

    public void deactivate()
    {
        if (isActive) {
            //Debug.Log("ITASK: Find lowest HP Enemy deactivated");
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    public void run(double deltaTime)
    {
        PlayerComponent opponent = null;
        foreach (PlayerComponent p in GameObject.FindObjectsOfType<PlayerComponent>()) {
            if (null != p.gameObject.GetComponent<inputSystem>()) {
                opponent = p;
            }
        }



        float minHP = Mathf.Infinity;
        AttributeComponent attributes;
        Dictionary<GameObject, float> values = new Dictionary<GameObject, float>();

        //Sammele alle Units mit ihren HP, merke dir die niedrigste HP für höchsten Priority Wert
        foreach(GameObject g in opponent.figurines) {
            if (!g)
                continue;
            attributes = g.GetComponent<AttributeComponent>();
            float hpValue = attributes.hp;

            if (hpValue < minHP)
                minHP = hpValue;

            values.Add(g, hpValue);
        }

        float actualHP;
        foreach(GameObject g in values.Keys) {
            actualHP = values[g];
            AIBlackboard.Instance.WriteToBlackboard(new WoundedEnemyNote(g, actualHP, minHP / actualHP));
            //Debug.Log("ITERATION: HP Status of Unit:" + g.name + " :" + actualHP + "   Priority:" + minHP / actualHP);
        }
        parent.finishChild(this, true);
        deactivate();
    }

    public void ResetNode() {
        deactivate();
    }
}
