using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MyBehaviour :IParentNode {

    IChildNode root;
    public bool isRunning;
    ArrayList activeTasks;
    ArrayList tasksToAdd;
    ArrayList tasksToRemove;

    bool behaviourTerminated;

    public MyBehaviour(IChildNode root)
    {
        activeTasks = new ArrayList();
        tasksToAdd = new ArrayList();
        tasksToRemove = new ArrayList();
        isRunning = false;
        this.root = root;
        behaviourTerminated = false;
    }

    public MyBehaviour() : this (null)
    {

    }

    public void activateTask(ITask task) {
        if (tasksToRemove.Contains(task))
            tasksToRemove.Remove(task);
        else
            tasksToAdd.Add(task);
    }

    public void deactivateTask(ITask task) {
        if (tasksToAdd.Contains(task))
            tasksToAdd.Remove(task);
        else
            tasksToRemove.Add(task);
    }

    public void Update(double deltaTime) {
        //if (!isRunning && !debug_terminated)
        //executeBehaviour();
        if (!isRunning)
            return;


        foreach (ITask task in tasksToRemove) {
            activeTasks.Remove(task);
        }
        tasksToRemove.Clear();

        foreach (ITask task in tasksToAdd) {
            activeTasks.Add(task);
        }
        tasksToAdd.Clear();

        foreach (ITask task in activeTasks)
            task.run(deltaTime);

        string debug = "";
        foreach(ITask tsk in activeTasks) {
            debug += tsk.ToString()+"\n";
            Utility.SetDebugText(debug);
        }
    }

    public void executeBehaviour() {
        if(null != root) {
            ResetChildren();

            activeTasks.Clear();
            tasksToAdd.Clear();
            tasksToRemove.Clear();


            isRunning = true;
            root.activate();
        }
    }

    public void AddChild(IChildNode child) {
        root = child;
    }

    public void finishChild(IChildNode child, bool result) {
        string finish = "Behaviour terminated with: " + result;
        Debug.Log(finish);
        Utility.SetDebugText(finish);
        isRunning = false;
        behaviourTerminated = true;
    }

    public bool hasBehaviourFinished() {
        if(behaviourTerminated) {
            behaviourTerminated = false;
            return true;
        }
        return false;
    }

    public void ResetChildren() {
        if(root != null)
            root.ResetNode();
    }
}
