using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WoundedEnemyNote :IBlackboardNote {

    private float priority;
    public bool isAssigned {
        get; set;
    }

    private GameObject unit;
    private float unitHP;

    public WoundedEnemyNote(GameObject data, float actualHP, float priority) {
        this.priority = priority;
        this.isAssigned = false;

        this.unit = data;
        this.unitHP = actualHP;
    }

    public Enums.BlackboardNotes getNoteType() {
        return Enums.BlackboardNotes.WEAKEST_ENEMY;
    }

    /// <summary>
    /// Gets the data of the Note
    /// ArrayList[0] = unit (GameObject)
    /// ArrayList[1] = unitHP (float)
    /// </summary>
    /// <returns>ArrayList with Data   index0 = unit  index1 = HPOfUnit</returns>
    public ArrayList getData() {
        ArrayList result = new ArrayList();
        result.Add(unit);
        result.Add(unitHP);

        return result;
    }

    public float getPriority() {
        return priority;
    }
}
