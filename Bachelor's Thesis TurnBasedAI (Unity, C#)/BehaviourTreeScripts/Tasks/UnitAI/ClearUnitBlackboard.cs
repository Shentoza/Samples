using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ClearUnitBlackboard : ITask {
    MyBehaviour behaviour;
    IParentNode parent;
    bool isActive;
    GameObject unit;

    public ClearUnitBlackboard(IParentNode parent, MyBehaviour behaviour, GameObject unit) {
        this.parent = parent;
        this.behaviour = behaviour;
        this.unit = unit;

        isActive = false;
    }

    public void activate() {
        Debug.Log("ITASK: Clear UnitBlackboard "+unit.name);
        isActive = true;
        behaviour.activateTask(this);

    }

    public void deactivate() {
        if (isActive) {
            Debug.Log("ITASK: Clear UnitBlackboard Done " + unit.name);
            isActive = false;
            behaviour.deactivateTask(this);
        }
    }

    public void run(double deltaTime) {
        UnitBlackboard blackboard = unit.GetComponent<UnitBlackboard>();
        AttributeComponent attributes = unit.GetComponent<AttributeComponent>();

        IBlackboardNote note = blackboard.getNote(Enums.BlackboardNotes.SAFEST_CELL);
        if(note != null && note.isAssigned) {
            if (!attributes.getCurrentCell().Equals((Cell)note.getData()[0]))
                note.isAssigned = false;
        }

        note = blackboard.getNote(Enums.BlackboardNotes.RETREAT_CELL);
        if (note != null && note.isAssigned) {
            if (!attributes.getCurrentCell().Equals((Cell)note.getData()[0]))
                note.isAssigned = false;
        }

        blackboard.Clear();

        deactivate();
        parent.finishChild(this, true);
    }

    public void ResetNode() {
        deactivate();
    }
}
