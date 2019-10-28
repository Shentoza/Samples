using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ParallelTask :IChildNode, IParentNode {
    List<IChildNode> children;
    ArrayList activeChildren;
    IParentNode parent;

    public ParallelTask(IParentNode parent) {
        this.parent = parent;
        children = new List<IChildNode>();
        activeChildren = new ArrayList();
    }

    public void activate() {
        foreach (IChildNode c in children) {
            activeChildren.Add(c);
            c.activate();
        }
    }

    public void AddChild(IChildNode child) {
        children.Add(child);
    }

    public void deactivate() {
        if (activeChildren.Count != 0) {
            foreach (IChildNode c in activeChildren)
                c.deactivate();

            activeChildren.Clear();
        }
    }

    public void finishChild(IChildNode child, bool result) {
        if (!result) {
            deactivate();
            parent.finishChild(this, false);
        }
        else {
            activeChildren.Remove(child);
            if (activeChildren.Count == 0) {
                Debug.Log("Paralleltask ended successfully!");
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
