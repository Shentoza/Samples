using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Limiter :IParentNode, IChildNode {
    IChildNode child;
    IParentNode parent;
    int limit;
    int timesActivated;
    UnitBlackboard blackboard;
    bool offensiveLimiter;

    public Limiter(IParentNode parent, int limit, UnitBlackboard blackboard = null, bool offensiveLimiter = false) {
        this.parent = parent;
        this.limit = limit;
        this.blackboard = blackboard;
        this.offensiveLimiter = offensiveLimiter;
    }

    public void activate() {
        if (blackboard) {
            //this.limit = offensiveLimiter ? blackboard.offensiveAP : blackboard.assignedAP;
            this.limit = blackboard.assignedAP;
        }
        if (null != child) {
            timesActivated = 0;
            child.activate();
        }
    }

    public void AddChild(IChildNode child) {
        if (null != this.child)
            return;
        this.child = child;
    }

    public void deactivate() {

    }

    public void finishChild(IChildNode child, bool result) {
        if (result) {
            timesActivated++;
            if (timesActivated < limit) {
                child.activate();
            }
            else
                parent.finishChild(this, true);
        }
        else
            parent.finishChild(this, false);
    }

    public void ResetNode() {
        timesActivated = 0;
        ResetChildren();
    }

    public void ResetChildren() {
        if(child != null)
            child.ResetNode();
    }
}
