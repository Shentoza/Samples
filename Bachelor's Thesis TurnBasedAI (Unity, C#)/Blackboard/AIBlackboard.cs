using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Prefab("Prefabs/Singletons/AI Blackboard", true)]
public class AIBlackboard :Singleton<AIBlackboard> {
    public List<IBlackboardNote> blackboardNotes;

    void Start() {
        blackboardNotes = new List<IBlackboardNote>();
    }

    public IBlackboardNote ReadFromBlackboard(Enums.BlackboardNotes type, bool getAssignedNotes = false) {
        /*
        //Sollte nicht notwendig sein, da vorher Sortiert wird
        float highestNotePriority = -1;
        IBlackboardNote highestNote = null;

        foreach (IBlackboardNote note in blackboardNotes) {
            if (note.getNoteType() == type && note.getPriority() > highestNotePriority) {
                highestNotePriority = note.getPriority();
                highestNote = note;
            }
        }
        return highestNote;
        */

        foreach (IBlackboardNote note in blackboardNotes)
            if (note.getNoteType() == type)
                if (getAssignedNotes)
                    return note;
                else
                    if(!note.isAssigned)
                        return note;

        return null;
    }

    public IBlackboardNote ReadFromBlackboard(bool getAssignedNotes = false) {
        /*
        float highestNotePriority = -1;
        IBlackboardNote highestNote = null;

        foreach(IBlackboardNote note in blackboardNotes) {
            if(note.getPriority() > highestNotePriority) {
                highestNote = note;
                highestNotePriority = note.getPriority();
            }
        }

        return highestNote;
        */
        //Sollte die höchste einzelne Note herausgeben, da es nach jedem Adden sortiert wird
        for (int i = 0; i < blackboardNotes.Count; ++i)
            if (getAssignedNotes)
                return blackboardNotes[i];
            else
                if(!blackboardNotes[i].isAssigned)
                    return blackboardNotes[i];
        return null;
    }

    public ArrayList GetAllNotesOfType(Enums.BlackboardNotes type, bool getAssignedNotes = false) {
        ArrayList value = new ArrayList();

        foreach (IBlackboardNote note in blackboardNotes) {
            if (note.getNoteType() == type) {
                if(getAssignedNotes) {
                    value.Add(note);
                }
                else {
                    if (!note.isAssigned) {
                        value.Add(note);
                    }
                }
            }

        }
        return value;
    }

    public void WriteToBlackboard(IBlackboardNote note) {
        Utility.SortNoteIntoList(note, blackboardNotes);
    }
}
