using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Data[0] = Zelle
/// Data[1] = hitChance
/// Data[2] = influence
/// 
/// Priority = (1-Hitchance*GewichtungHitchance + Einfluss(0..1)*GewichtungEinfluss)²
/// </summary>
public class SafestCellNote :IBlackboardNote {
    private float priority;
    public bool isAssigned {
        get; set;
    }

    private float hitChance;
    private float influence;
    private Cell cell;

    public SafestCellNote(float priority, Cell cell, float hitChance, float influence) {
        this.priority = priority;
        this.isAssigned = false;

        this.cell = cell;
        this.hitChance = hitChance;
        this.influence = influence;
    }

    public SafestCellNote(ArrayList data) : this( (float) data[0], (Cell) data[1], (float) data[2], (float) data[3]) {

    }

    public ArrayList getData() {
        ArrayList result = new ArrayList();
        result.Add(cell);
        result.Add(hitChance);
        result.Add(influence);

        return result;
    }

    public Enums.BlackboardNotes getNoteType() {
        return Enums.BlackboardNotes.SAFEST_CELL;
    }

    public float getPriority() {
        return priority;
    }
}
