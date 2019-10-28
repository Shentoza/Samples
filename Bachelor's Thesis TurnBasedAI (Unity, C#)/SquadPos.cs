using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SquadPos : MonoBehaviour {

    public ArrayList figurines;
    public PlayerComponent p1;
	// Use this for initialization
	void Start () {
	}
	
	// Update is called once per frame
	void Update () {
        if (figurines == null)
            figurines = p1.figurines;
        if (figurines == null)
            return;
        Vector3 avgPos = new Vector3();
        if(figurines.Count > 0) {
		    foreach(GameObject f in figurines) {
                avgPos += f.GetComponent<AttributeComponent>().getCurrentCell().transform.position;
            }
            avgPos /= figurines.Count;
        }
        transform.position = avgPos;
	}
}
