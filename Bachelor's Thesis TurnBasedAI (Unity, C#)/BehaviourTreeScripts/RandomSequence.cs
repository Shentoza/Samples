using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RandomSequence :IChildNode, IParentNode {
    List<IChildNode> children;
    IParentNode parent;
    int currentChildIndex;


    public RandomSequence(IParentNode parent) {
        this.parent = parent;
        children = new List<IChildNode>();
        currentChildIndex = -1;
    }

    public void activate() {
        Utility.randomizeList<IChildNode>(children);
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
            parent.finishChild(this, false);
        }
        else {
            currentChildIndex++;
            if (currentChildIndex < children.Count) {
                children[currentChildIndex].activate();
            }
            else {
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
