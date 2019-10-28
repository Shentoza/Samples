using UnityEngine;
using System.Collections;

public class AIController : MonoBehaviour {

    MyBehaviour strategicBehaviour;
    public bool player1;
    PlayerComponent player;

    bool initialized = false;

    bool processChildren = false;

    int currentChildIndex = 0;
    UnitAIController currentAI;

    int APperUnit;
    int APLeft;

    void Start() {
        if (player1)
            player = GameObject.Find("Player1").GetComponent<PlayerComponent>();
        else
            player = GameObject.Find("Player2").GetComponent<PlayerComponent>();
        //Strategisches Verhalten von der Gesamt KI
        strategicBehaviour = new MyBehaviour();
        Sequence generalAISequence = new Sequence(strategicBehaviour);
        strategicBehaviour.AddChild(generalAISequence);

        Selector scoreEnemyUnits = new Selector(generalAISequence);
        generalAISequence.AddChild(scoreEnemyUnits);
		Inverter invEnemies = new Inverter(scoreEnemyUnits);
		AreThereEnemies enemiesLeftNote = new AreThereEnemies(invEnemies, strategicBehaviour);
		invEnemies.AddChild(enemiesLeftNote);
		
        FindLowestHPEnemy checkEnemyHP = new FindLowestHPEnemy(scoreEnemyUnits, strategicBehaviour);
        scoreEnemyUnits.AddChild(invEnemies);
        scoreEnemyUnits.AddChild(checkEnemyHP);

        Sequence scoreCells = new Sequence(generalAISequence);
        generalAISequence.AddChild(scoreCells);

        EvaluateSafeCells findSafeCells = new EvaluateSafeCells(scoreCells, strategicBehaviour);
        EvaluateRetreatCells findRetreatCells = new EvaluateRetreatCells(scoreCells, strategicBehaviour);
        EvaluateOffensiveCells offensiveCells = new EvaluateOffensiveCells(scoreCells, strategicBehaviour);

        scoreCells.AddChild(findSafeCells);
        scoreCells.AddChild(findRetreatCells);
        scoreCells.AddChild(offensiveCells);


        initialized = true;
    }

    //Du bist am Zug!
    void OnEnable()
    {
        StartCoroutine(StartAfterInit());
    }


    public IEnumerator StartAfterInit() {
        yield return new WaitUntil(() => initialized);
        processChildren = false;
        currentChildIndex = 0;
        currentAI = null;

        foreach(GameObject g in player.figurines) {
			if(!g)
				continue;
            UnitAIController controller = g.GetComponent<UnitAIController>();
            if (controller)
                controller.turnDone = false;
        }

        strategicBehaviour.executeBehaviour();
    }

    void Update() {
        strategicBehaviour.Update(Time.deltaTime);
        if (strategicBehaviour.hasBehaviourFinished()) {
            processChildren = true;
            CalculateChildrenAP();
            activateNextUnitAI();
        }
        if (!processChildren)
            return;

        if (!currentAI.initialized) {
            return;
        }
        if (currentAI.turnDone) {
            //Alle AIs fertig
            if(currentChildIndex == player.figurines.Count)
                EndTurn();
            else {
                processChildren = false;
                strategicBehaviour.executeBehaviour();
            }
        }

    }

    void activateNextUnitAI() {
        if(currentChildIndex == player.figurines.Count) {
            EndTurn();
            return;
        }

        GameObject unit = (GameObject) player.figurines[currentChildIndex];
        if(unit == null) {
            currentChildIndex++;
            activateNextUnitAI();
        }
        else {
            Debug.Log("Next Child!");
            currentAI = unit.GetComponent<UnitAIController>();
            currentAI.enabled = true;
            currentChildIndex++;
        }
    }


    void EndTurn() {
        AIBlackboard.Instance.blackboardNotes.Clear();
        EndturnEvent.Send(!player1);
        processChildren = false;
    }

    void CalculateChildrenAP() {
        int childrenLeft = player.figurines.Count - currentChildIndex;
        APperUnit = player.actionPoints / childrenLeft;
        APLeft = player.actionPoints % childrenLeft;
        foreach(GameObject g in player.figurines) {
			if(!g)
			{
				continue;
			}
            if (player.figurines.IndexOf(g) < currentChildIndex)
                continue;
            UnitBlackboard blackboard = g.GetComponent<UnitBlackboard>();
            blackboard.assignedAP = APperUnit;
            if (APLeft > 0) {
                blackboard.assignedAP++;
                APLeft--;
            }
            blackboard.offensiveAP = blackboard.assignedAP / 2;
        }
    }
}
