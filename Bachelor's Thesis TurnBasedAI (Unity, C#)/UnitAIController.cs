using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UnitAIController : MonoBehaviour {
    public MyBehaviour unitBehaviour;
    public bool turnDone { get; set; }
    public bool initialized = false;

    void Start() {

        GameObject unit = this.gameObject;
        UnitBlackboard blackboard = unit.GetComponent<UnitBlackboard>();
        int team = unit.GetComponent<AttributeComponent>().team;
        PlayerComponent player = null, opponent = null;
        if(team == 1) {
            player = GameObject.Find("Player1").GetComponent<PlayerComponent>();
            opponent = GameObject.Find("Player2").GetComponent<PlayerComponent>();
        }
        else if(team == 2) {
            player = GameObject.Find("Player2").GetComponent<PlayerComponent>();
            opponent = GameObject.Find("Player1").GetComponent<PlayerComponent>();
        }
        else {
            Debug.Log("No Player found :O");
        }
        //Behaviour einer einzelnen Einheit



        unitBehaviour = new MyBehaviour();
        Sequence evalThenStuff = new Sequence(unitBehaviour);
        unitBehaviour.AddChild(evalThenStuff);


        FillUnitBlackboard fillUnitBlackboard = new FillUnitBlackboard(evalThenStuff, unitBehaviour, unit, opponent);
        Selector woundedOrNot = new Selector(evalThenStuff);
        ClearUnitBlackboard clearBlackboard = new ClearUnitBlackboard(evalThenStuff, unitBehaviour, unit);

        evalThenStuff.AddChild(fillUnitBlackboard);
        evalThenStuff.AddChild(woundedOrNot);
        evalThenStuff.AddChild(clearBlackboard);

        //Nicht verwundet
        Sequence notWoundedSequence = new Sequence(woundedOrNot);
        woundedOrNot.AddChild(notWoundedSequence);

        Inverter invertWounded = new Inverter(notWoundedSequence);
        AmIWounded woundedCheck = new AmIWounded(invertWounded, unitBehaviour, unit, 30);
        invertWounded.AddChild(woundedCheck);
        Selector canIShootOrGoToSafe = new Selector(notWoundedSequence);

        notWoundedSequence.AddChild(invertWounded);
        notWoundedSequence.AddChild(canIShootOrGoToSafe);

        Sequence shooting = new Sequence(canIShootOrGoToSafe);
        canIShootOrGoToSafe.AddChild(shooting);

        //Knotenabfolge für Bewegung zur Safecell
        Inverter    moveAfterShot_ResultInv  = new Inverter(canIShootOrGoToSafe);
        Limiter     moveAfterShot_Limiter    = new Limiter(moveAfterShot_ResultInv, 1, blackboard);
        Inverter    moveAfterShot_MoveInv    = new Inverter(moveAfterShot_Limiter);
        MoveToCell  moveAfterShot_Task       = new MoveToCell(moveAfterShot_MoveInv, unitBehaviour, unit, player, Enums.BlackboardNotes.SAFEST_CELL);
        moveAfterShot_ResultInv.AddChild (moveAfterShot_Limiter);
        moveAfterShot_Limiter.AddChild   (moveAfterShot_MoveInv);
        moveAfterShot_MoveInv.AddChild   (moveAfterShot_Task);


        canIShootOrGoToSafe.AddChild(moveAfterShot_ResultInv);
        canIShootOrGoToSafe.AddChild(new AlwaysTrue(canIShootOrGoToSafe));

        CanIShoot shootingCheck = new CanIShoot(shooting, unitBehaviour, this.gameObject);

        //Knotenabfolge für Bewegung zur Angriffszelle
        Inverter    moveToShootPos_ResultInv =  new Inverter(shooting);
        Limiter     moveToShootPos_Limiter   =  new Limiter(moveToShootPos_ResultInv, 1, blackboard, true);
        Inverter    moveToShootPos_MoveInv   =  new Inverter(moveToShootPos_Limiter);
        MoveToCell  moveToShootPos_Task      =  new MoveToCell(moveToShootPos_MoveInv, unitBehaviour, unit, player, Enums.BlackboardNotes.OFFENSIVE_CELL);
        moveToShootPos_ResultInv.AddChild(moveToShootPos_Limiter);
        moveToShootPos_Limiter.AddChild(moveToShootPos_MoveInv);
        moveToShootPos_MoveInv.AddChild(moveToShootPos_Task);


        ShootAtTarget shotTask = new ShootAtTarget(shooting, unitBehaviour, unit, player);
        Inverter alwaysFalse = new Inverter(shooting);
        AlwaysTrue true_shooting = new AlwaysTrue(alwaysFalse);
        alwaysFalse.AddChild(true_shooting);

        shooting.AddChild(shootingCheck);
        shooting.AddChild(moveToShootPos_ResultInv);
        shooting.AddChild(shotTask);
        shooting.AddChild(alwaysFalse);

        

        //Er ist verwundet! Rechte Hälfte des Baumes
        Sequence retreatSequence = new Sequence(woundedOrNot);
        woundedOrNot.AddChild(retreatSequence);

        Selector DesperationHit_Selector = new Selector(retreatSequence);
        retreatSequence.AddChild(DesperationHit_Selector);

        Inverter DesperationHit_Inverter = new Inverter(DesperationHit_Selector);
        GetDesperationHit DesperationHit_Check = new GetDesperationHit(DesperationHit_Inverter, unitBehaviour, unit, opponent);
        ShootAtTarget desperationShot = new ShootAtTarget(DesperationHit_Selector, unitBehaviour, unit, player);

        DesperationHit_Inverter.AddChild(DesperationHit_Check);

        DesperationHit_Selector.AddChild(DesperationHit_Inverter);
        DesperationHit_Selector.AddChild(desperationShot);


        //Knotenabfolge für Bewegung zur Rückzugszelle
        Inverter    goToRetreatCell_ResultInv = new Inverter(retreatSequence);
        Limiter     goToRetreatCell_Limiter   = new Limiter(goToRetreatCell_ResultInv, 1, blackboard);
        Inverter    goToRetreatCell_MoveInv   = new Inverter(goToRetreatCell_Limiter);
        MoveToCell  goToRetreatCell_Task      = new MoveToCell(goToRetreatCell_MoveInv, unitBehaviour, unit, player, Enums.BlackboardNotes.RETREAT_CELL);
        goToRetreatCell_ResultInv.AddChild(goToRetreatCell_Limiter);
        goToRetreatCell_Limiter.AddChild(goToRetreatCell_MoveInv);
        goToRetreatCell_MoveInv.AddChild(goToRetreatCell_Task);

        retreatSequence.AddChild(goToRetreatCell_ResultInv);

        initialized = true;
    }

    //Du bist am Zug!
    void OnEnable() {
        StartCoroutine(StartAfterInit());
    }

    //Zug abgeben
    void OnDisable() {
    }

    void Update() {
        unitBehaviour.Update(Time.deltaTime);
        if (unitBehaviour.hasBehaviourFinished()) {
            this.enabled = false;
            this.turnDone = true;
        }
    }


    public IEnumerator StartAfterInit() {
        yield return new WaitUntil(() => initialized);

        turnDone = false;
        unitBehaviour.executeBehaviour();
    }
}
