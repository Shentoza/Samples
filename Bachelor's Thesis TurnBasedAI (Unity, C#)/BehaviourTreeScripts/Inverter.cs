using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Inverter :IChildNode, IParentNode {
    IChildNode child;
    IParentNode parent;
    string name;

    public Inverter(IParentNode parent, string name = null) {
        if (name != null)
            this.name = name;
        this.parent = parent;
    }

    public void activate() {
        if (null != this.child) {
            if(name != null)
                Debug.Log("INVERTER "+ name+ "activated");
            child.activate();
        }
    }

    public void AddChild(IChildNode child) {
        if (this.child != null)
            return;
        this.child = child;
    }

    public void deactivate() {

    }


    public void finishChild(IChildNode child, bool result) {
        if (name != null)
            Debug.Log("INVERTER " + name+ " :  Result was " + result + " now its " + !result);

        parent.finishChild(this, !result);
    }

    public void ResetNode() {
        ResetChildren();
    }

    public void ResetChildren() {
        if(child != null)
            child.ResetNode();
    }
}
