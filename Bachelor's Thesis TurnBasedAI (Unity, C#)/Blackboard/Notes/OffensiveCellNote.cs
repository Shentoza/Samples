using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Data[0] = Zelle
/// Data[1] = hitChance
/// Data[2] = tension (float)
/// Data[3] = target (GameObject)
/// Priority = (Hitchance*GewichtungHitchance + Tension(0..1)*GewichtungTension)²
/// </summary>
public class OffensiveCellNote :IBlackboardNote {
    private float priority;
    public bool isAssigned {
        get; set;
    }

    private float hitChance;
    private float tension;
    private Cell cell;
    private GameObject target;

    public OffensiveCellNote(float priority, Cell cell, float hitChance, float tension, GameObject unit) {
        this.priority = priority;
        this.isAssigned = false;

        this.cell = cell;
        this.hitChance = hitChance;
        this.tension = tension;
        this.target = unit;
    }

    public OffensiveCellNote(ArrayList data) : this((float)data[0],(Cell)data[1], (float) data[2], (float) data[3], (GameObject) data[4]) {

    }

    /// <summary>
    /// ArrayList[0] = Cell (Cell)
    /// ArrayList[1] = hitChance (float)
    /// ArrayList[2] = tension (float)
    /// ArrayList[3] = target (GameObject)
    /// </summary>
    /// <returns></returns>
    public ArrayList getData() {
        ArrayList result = new ArrayList();
        result.Add(cell);
        result.Add(hitChance);
        result.Add(tension);
        result.Add(target);

        return result;
    }

    public Enums.BlackboardNotes getNoteType() {
        return Enums.BlackboardNotes.OFFENSIVE_CELL;
    }

    public float getPriority() {
        return priority;
    }
}
