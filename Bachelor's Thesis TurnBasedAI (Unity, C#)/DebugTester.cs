using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DebugTester : MonoBehaviour {
    bool toggleInfluence = false;
    bool toggleTension = false;
    bool toggleVulnerability = false;
    bool toggleDirectedVulnerability = false;

    public GameObject overviewCam;
    public CameraRotationScript standardCam;
    // Update is called once per frame
    void Update () {
        //InfluenceMap
        if(Input.GetKeyDown("i")) {
            toggleDirectedVulnerability = toggleTension = toggleVulnerability = false;
            toggleInfluence = !toggleInfluence;
            showInfluenceMap(toggleInfluence);
            checkForInputAndCam();


        }
        //TensionMap
        if (Input.GetKeyDown("t")) {
            toggleDirectedVulnerability = toggleVulnerability = toggleInfluence = false;
            toggleTension = !toggleTension;
            showTensionMap(toggleTension);
            checkForInputAndCam();
        }

        //VulnerabilityMap
        if (Input.GetKeyDown("v")) {
            toggleDirectedVulnerability= toggleTension = toggleInfluence = false;
            toggleVulnerability = !toggleVulnerability;
            showVulnerabilityMap(toggleVulnerability);
            checkForInputAndCam();
        }

        //Directed Vulnerability
        if (Input.GetKeyDown("d")) {
            toggleVulnerability = toggleTension = toggleInfluence = false;
            toggleDirectedVulnerability = !toggleDirectedVulnerability;
            showDirectedVulnerabilityMap(toggleDirectedVulnerability);
            checkForInputAndCam();
        }

        //Calculate Values
        if (Input.GetKeyDown("c")) {
            InfluenceMap.Instance.calculateTeamInfluence(1);
            InfluenceMap.Instance.calculateTeamInfluence(2);
        }
	}

    public void checkForInputAndCam() {
        if(toggleInfluence || toggleTension || toggleVulnerability || toggleDirectedVulnerability) {
            if(!overviewCam.activeInHierarchy)
                overviewCam.SetActive(true);
            
            if(standardCam.enabled) {
                standardCam.enabled = false;
            }
        }
        else {
            if (overviewCam.activeInHierarchy)
                overviewCam.SetActive(false);

            if (!standardCam.enabled) {
                standardCam.enabled = true;
            }

        }
    }

    public void showInfluenceMap(bool on) {
        if (!on) {
            PlayerAssistanceSystem.Instance.resetAllCellColors();
            return;
        }
        float[,] infl = InfluenceMap.Instance.generateInfluenceMap(2);
        PlayerAssistanceSystem.Instance.VisualizeInfluenceMap(infl, InfluenceMap.Instance.maxInfluence);
    }

    public void showTensionMap(bool on) {
        if (!on) {
            PlayerAssistanceSystem.Instance.resetAllCellColors();
            return;
        }
        float[,] infl = InfluenceMap.Instance.generateTensionMap();
        PlayerAssistanceSystem.Instance.VisualizeInfluenceMap(infl,InfluenceMap.Instance.maxTension);
    }

    public void showVulnerabilityMap(bool on) {
        if (!on) {
            PlayerAssistanceSystem.Instance.resetAllCellColors();
            return;
        }
        float[,] infl = InfluenceMap.Instance.generateVulnerabilityMap();
        PlayerAssistanceSystem.Instance.VisualizeInfluenceMap(infl, InfluenceMap.Instance.maxInfluence);
    }

    public void showDirectedVulnerabilityMap(bool on) {
        if (!on) {
            PlayerAssistanceSystem.Instance.resetAllCellColors();
            return;
        }
        float[,] infl = InfluenceMap.Instance.generateDirectedVulnerabilityMap(2);
        PlayerAssistanceSystem.Instance.VisualizeInfluenceMap(infl, InfluenceMap.Instance.maxInfluence);
    }

}
