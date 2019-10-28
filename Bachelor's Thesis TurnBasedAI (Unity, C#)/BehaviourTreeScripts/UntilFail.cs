using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UntilFail :IParentNode, IChildNode {
    IChildNode child;
    IParentNode parent;

    public UntilFail(IParentNode parent) {
        child = null;
        this.parent = parent;
    }

    public void activate() {
        if (null != child)
            child.activate();
    }

    public void AddChild(IChildNode child) {
        if (null != child)
            return;
        this.child = child;
    }

    public void deactivate() {
    }

    public void finishChild(IChildNode child, bool result) {
        if (result)
            child.activate();
        else
            parent.finishChild(this, false);
    }

    public void ResetNode() {
        ResetChildren();
    }

    public void ResetChildren() {
        child.ResetNode();
    }

}
