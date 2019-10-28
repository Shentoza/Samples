using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UnitBlackboard : MonoBehaviour {
    public IBlackboardNote offensivePosition { get; set; }
    public IBlackboardNote safePosition { get; set; }
    public IBlackboardNote retreatPosition { get; set; }
    public GameObject targetUnit { get; set; }
    public int assignedAP { get; set; }
    public int offensiveAP { get ; set; }
    public Cell adjustedShotCell { get; set; }

    public IBlackboardNote getNote(Enums.BlackboardNotes type) {
        switch (type) {
            case Enums.BlackboardNotes.OFFENSIVE_CELL:
             return offensivePosition;
            case Enums.BlackboardNotes.RETREAT_CELL:
                return retreatPosition;
            case Enums.BlackboardNotes.SAFEST_CELL:
                return safePosition;
            default:
                return null;
        }
    }

    public void Clear() {
        offensivePosition = safePosition = retreatPosition = null;
        targetUnit = null;
        assignedAP = offensiveAP = 0;
    }
}
