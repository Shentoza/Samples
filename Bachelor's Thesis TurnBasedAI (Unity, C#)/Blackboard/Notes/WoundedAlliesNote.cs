using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WoundedAlliesNote :IBlackboardNote {

    private float priority;
    public bool isAssigned {
        get; set;
    }

    private GameObject unit;
    private float unitHP;

    public WoundedAlliesNote(GameObject data, float actualHP, float priority) {
        this.priority = priority;
        this.isAssigned = false;

        this.unit = data;
        this.unitHP = actualHP;
    }

    public Enums.BlackboardNotes getNoteType() {
        return Enums.BlackboardNotes.WEAKEST_ALLY;
    }

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
