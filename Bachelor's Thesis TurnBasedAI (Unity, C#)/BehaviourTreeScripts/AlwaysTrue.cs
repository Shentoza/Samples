using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AlwaysTrue :IChildNode {
    IParentNode parent;

    public AlwaysTrue(IParentNode parent) {
        this.parent = parent;
    }

    public void activate() {
        parent.finishChild(this, true);
    }

    public void deactivate() {

    }

    public void ResetNode() {
		deactivate();
    }
}
