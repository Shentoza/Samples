using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Selector :IParentNode, IChildNode {
    string DEBUG_NAME;

    List<IChildNode> children;
    IParentNode parent;
    int currentChildIndex;


    public Selector(IParentNode parent, string DEBUG_NAME = null) {
        this.parent = parent;
        children = new List<IChildNode>();
        currentChildIndex = -1;

        this.DEBUG_NAME = DEBUG_NAME;
    }

    public void activate() {
        if (DEBUG_NAME != null)
            Debug.Log("SELECTOR " + DEBUG_NAME + " : activated ");
        if (children.Count > 0) {
            currentChildIndex = 0;
            children[0].activate();
        }
    }

    public void deactivate() {
    }

    public void AddChild(IChildNode child) {
        children.Add(child);
    }

    public void finishChild(IChildNode child, bool result) {
        if (result) {
            parent.finishChild(this, true);
            if (DEBUG_NAME != null)
                Debug.Log("SELECOTR " + DEBUG_NAME + "     :FERTIG DURCH" + child.GetType());
        }
        else {
            currentChildIndex++;
            if (currentChildIndex < children.Count) {
                children[currentChildIndex].activate();
                if (DEBUG_NAME != null) {
                    Debug.Log("SELECTOR " + DEBUG_NAME + "  : OK durch   " + child.GetType());
                    Debug.Log("SELECTOR " + DEBUG_NAME + "  : JETZT:   " + children[currentChildIndex].GetType());
                }

            }
            else {
                parent.finishChild(this, false);
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
