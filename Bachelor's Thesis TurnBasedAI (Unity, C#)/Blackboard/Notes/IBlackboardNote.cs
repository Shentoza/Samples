using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface IBlackboardNote {


    bool isAssigned { get; set; }
    float getPriority();
    Enums.BlackboardNotes getNoteType();
    ArrayList getData();
}