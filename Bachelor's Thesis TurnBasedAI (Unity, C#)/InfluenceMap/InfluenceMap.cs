using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Prefab("Prefabs/Singletons/Influence Map", true)]
public class InfluenceMap : Singleton<InfluenceMap> {
    private float m_baseInfluencyValue = 5.0f;
    private float m_decayRate = 0.5f;


    public float maxTension {
        get {
            return 2.0f * (m_baseInfluencyValue - m_decayRate);
        }
    }
    public float maxInfluence {
        get {
            return m_baseInfluencyValue;
        }
    }

    
    float[,] influenceTeam1;
    float[,] influenceTeam2;

    ArrayList influencersTeam1;
    ArrayList influencersTeam2;

    private int sizeX = -1;
    private int sizeZ = -1;

    // Use this for initialization
    void Awake()
    {
        sizeX = BattlefieldCreater.mapSizeX;
        sizeZ = BattlefieldCreater.mapSizeZ;
        influenceTeam1 = new float[sizeX, sizeZ];
        influenceTeam2 = new float[sizeX, sizeZ];
    }

    /// <summary>
    /// Generiert Influence Map
    /// Team Influence - Opposing Team Influence
    /// It will produce a high+ value if the own value is superior, and produce a high- value if the enemy value is stronger
    /// </summary>
    /// <param name="team">Number for which team the influence shall be calculated (only 1 or 2 possible currently)</param>
    public float[,] generateInfluenceMap(int team = 1)
    {
        float[,] a;
        float[,] b;
        if (team == 1)
        {
            a = influenceTeam1;
            b = influenceTeam2;
        }
        else if (team == 2)
        {
            a = influenceTeam2;
            b = influenceTeam1;
        }
        else
            return null;

        float[,] result = new float[sizeX, sizeZ];

        for (int x = 0; x < sizeX; ++x)
            for (int z = 0; z < sizeZ; ++z)
                result[x, z] = a[x, z] - b[x, z];

        return result;
    }

    /// <summary>
    /// Generiert Tension Map
    /// = TeamInfluence + OpposingTeamInfluence
    /// It will produce a high value where both teams are strong, and a low value where both teams are weak
    /// </summary>
    public float[,] generateTensionMap()
    {
        float[,] a = influenceTeam1;
        float[,] b = influenceTeam2;

        float[,] result = new float[sizeX, sizeZ];

        for (int x = 0; x < sizeX; ++x)
            for (int z = 0; z < sizeZ; ++z) {
                result[x, z] = a[x, z] + b[x, z];
            }

        return result;
    }

    /// <summary>
    /// Generiert VulnerabilityMap
    /// = TensionMap - Abs(InfluenceMap)
    /// It will produce a high value where there is high tension but the sides are pretty equal while it will produce a lower value where there is high tension but one side is much superior.
    /// </summary>
    public float[,] generateVulnerabilityMap()
    {
        float[,] a = generateTensionMap();
        float[,] b = generateInfluenceMap();

        float[,] result = new float[sizeX, sizeZ];

        for (int x = 0; x < sizeX; ++x)
            for (int z = 0; z < sizeZ; ++z)
                result[x, z] = a[x, z] - Mathf.Abs(b[x, z]);

        return result;
    }

    /// <summary>
    /// Generiert VulnerabilityMap
    /// = TensionMap+InfluenceMap(team)
    /// It will give high values in area of conflict where you are strong and low values in areas of conflict where you are weak.
    /// </summary>
    public float[,] generateDirectedVulnerabilityMap(int team) {
        if (team < 1 || team > 2)
            return null;

        float[,] a = generateTensionMap();
        float[,] b = generateInfluenceMap(team);

        float[,] result = new float[sizeX, sizeZ];
        for (int x = 0; x < sizeX; ++x)
            for (int z = 0; z < sizeZ; ++z)
                result[x, z] = a[x, z] + b[x, z];

        return result;
    }

    /// <summary>
    /// Calculates the influence map for a given Team.
    /// Calls Dijkstra Execute method, so be sure to call it again afterwards
    /// </summary>
    /// <param name="team">Number for which team the influence shall be calculated (only 1 or 2 possible currently)</param>
    public void calculateTeamInfluence(int team)
    {
        if (team < 1 || team > 2)
            return;

        ArrayList currentInfluencers = null;
        float[,] currentInfluenceMap = null;
        if(team == 1)
        {
            if(influencersTeam1 == null)
            {
                influencersTeam1 = GameObject.Find("Player1").GetComponent<PlayerComponent>().figurines;
            }
            currentInfluencers = influencersTeam1;
            currentInfluenceMap = influenceTeam1;
        }
        else if(team == 2)
        {
            if (influencersTeam2 == null)
            {
                influencersTeam2 = GameObject.Find("Player2").GetComponent<PlayerComponent>().figurines;
            }
            currentInfluencers = influencersTeam2;
            currentInfluenceMap = influenceTeam2;
        }
        if (currentInfluencers == null)
        {
            Debug.Log("No influencers for Team " + team);
            return;
        }
        Array.Clear(currentInfluenceMap, 0, currentInfluenceMap.Length);
        Cell currentCell;

        foreach(GameObject unit in currentInfluencers) {
			if(!unit)
				continue;
			
            currentCell = unit.GetComponent<AttributeComponent>().getCurrentCell();
            currentInfluenceMap[currentCell.xCoord, currentCell.zCoord] = m_baseInfluencyValue;
            DijkstraSystem.Instance.executeDijsktra(currentCell);
            for(int x = 0; x < sizeX; ++x)
                for(int z = 0; z < sizeZ; ++z)
                {
                    float maxValue = Mathf.Max(0.0f, currentInfluenceMap[x,z]);
                    currentCell = BattlefieldCreater.instance.getCell(x, z);
                    if(!currentCell.isOccupied)
                        maxValue = Mathf.Max(maxValue, currentInfluenceMap[x,z], m_baseInfluencyValue - (currentCell.dij_GesamtKosten * m_decayRate));

                    currentInfluenceMap[x, z] = maxValue;
                }
        }
    }
}
