using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Sequence :IChildNode, IParentNode {
    string DEBUG_NAME;
    List<IChildNode> children;
    IParentNode parent;
    int currentChildIndex;


    public Sequence(IParentNode parent, string DEBUG_NAME = null) {
        this.parent = parent;
        children = new List<IChildNode>();
        currentChildIndex = -1;
        this.DEBUG_NAME = DEBUG_NAME;
    }

    public void activate() {
        if (DEBUG_NAME != null)
            Debug.Log("SEQUENCE " + DEBUG_NAME + "  : activated ");
        if (children.Count > 0) {
            currentChildIndex = 0;
            children[0].activate();
        }
    }

    public void AddChild(IChildNode child) {
        children.Add(child);
    }

    public void deactivate() {
    }

    public void finishChild(IChildNode child, bool result) {
        if (!result) {
            if (DEBUG_NAME != null)
                Debug.Log("SEQUENCE " + DEBUG_NAME + "  : FAILED durch   "+ child.GetType());
            parent.finishChild(this, false);
        }
        else {
            currentChildIndex++;
            if (currentChildIndex < children.Count) {
                if (DEBUG_NAME != null) {
                    Debug.Log("SEQUENCE " + DEBUG_NAME + "  : OK durch   " + child.GetType());
                    Debug.Log("SEQUENCE " + DEBUG_NAME + "  : JETZT:   " + children[currentChildIndex].GetType());
                }

                children[currentChildIndex].activate();
            }
            else {
                if (DEBUG_NAME != null) {
                    Debug.Log("SEQUENCE " + DEBUG_NAME + "  : Alle fertig durch:    "+child.GetType());
                }
                    parent.finishChild(this, true);
            }
        }
    }

    public void ResetNode() {
        ResetChildren();
    }

    public void ResetChildren() {
        foreach (IChildNode child in children)
            child.ResetNode();
    }
}
