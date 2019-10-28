using System.Collections;
using UnityEngine;

public interface IParentNode {
    void AddChild(IChildNode child);
    void finishChild(IChildNode child, bool result);
    void ResetChildren();
}

public interface IChildNode {
    void activate();
    void deactivate();
    void ResetNode();
}

public interface ITask :IChildNode {
    void run(double deltaTime);
}
