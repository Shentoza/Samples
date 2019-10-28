using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Data[0] = Zelle
/// Data[1] = hitChance
/// Data[2] = lowTension + highOwnInfluence
/// 
/// Priority = (Hitchance*GewichtungHitchance + Tension(0..1)*GewichtungTension)²
/// </summary>
public class RetreatCellNote :IBlackboardNote {
    private float priority;
    public bool isAssigned {
        get; set;
    }

    private float hitChance;
    private float tension;
    private Cell cell;

    public RetreatCellNote(float priority, Cell cell, float hitChance, float tension) {
        this.priority = priority;
        this.isAssigned = false;

        this.cell = cell;
        this.hitChance = hitChance;
        this.tension = tension;
    }

    public RetreatCellNote(ArrayList data) : this( (float) data[0], (Cell) data[1], (float) data[2], (float) data[3]) {

    }

    public ArrayList getData() {
        ArrayList result = new ArrayList();
        result.Add(cell);
        result.Add(hitChance);
        result.Add(tension);

        return result;
    }

    public Enums.BlackboardNotes getNoteType() {
        return Enums.BlackboardNotes.RETREAT_CELL;
    }

    public float getPriority() {
        return priority;
    }
}
