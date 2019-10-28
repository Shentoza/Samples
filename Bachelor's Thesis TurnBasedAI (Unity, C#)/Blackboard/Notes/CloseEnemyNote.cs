using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


/// <summary>
/// Data[0] = enemyUnit
/// Data[1] = Liste der eigenen Units in aufsteigender Distanz
/// </summary>
public class CloseEnemyNote :IBlackboardNote {
    private float priority;
    public bool isAssigned {
        get; set;
    }

    private GameObject enemyUnit;
    private ArrayList ourUnitList;

    public CloseEnemyNote(GameObject enemyUnit, ArrayList units, float priority) {
        //Basic Initialisation
        isAssigned = false;
        this.priority = priority;

        //Data
        this.enemyUnit = enemyUnit;
        this.ourUnitList = units;
    }

    public Enums.BlackboardNotes getNoteType() {
        return Enums.BlackboardNotes.CLOSE_ENEMY;
    }

    public ArrayList getData() {
        ArrayList result = new ArrayList();
        result.Add(enemyUnit);
        result.Add(ourUnitList);

        return result;
    }

    public float getPriority() {
        return priority;
    }
}
