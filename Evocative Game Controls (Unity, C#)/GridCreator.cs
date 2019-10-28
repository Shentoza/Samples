using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GridCreator :MonoBehaviour {
    public static GridCreator Instance;

    public Rect levelSize;

    [SerializeField]
    private float[] offsets;
    [SerializeField]
    private bool initiliase = true;

    public GameObject cell;
    private ArrayList m_cells;
    public ArrayList CellList {
        get {
            return m_cells;
        }
    }

    public string[,] baseKeys = new string[,] {
        { "1", "2", "3", "4", "5", "6", "7", "8", "9" },
        { "q", "w", "e", "r", "t", "z", "u", "i", "o" },
        { "a", "s", "d", "f", "g", "h", "j", "k", "l" },
        { "y", "x", "c", "v", "b", "n", "m", ",", "." }
    };

    public string[,] scrambledKeys = new string[,] {
        { "9", "8", "7", "6", "5", "4", "3", "2", "1" },
        { "o", "i", "u", "z", "t", "r", "e", "w", "q" },
        { "l", "k", "j", "h", "g", "f", "d", "s", "a" },
        { ".", ",", "m", "n", "b", "v", "c", "x", "y" }
    };

    // Use this for initialization
    void Start () {
        if (Instance == null) Instance = this;
        else Destroy(this);

        m_cells = new ArrayList();

        // init and create the tiles
        if (initiliase)
        {
            //Dimensions because 1x1x1 Plane is actually 10 long
            //magic numbers, yay
            float cellDimensionX = levelSize.width / 9.0f;
            float cellDimensionY = levelSize.height / 4.0f;

            int count = 0;
            for(int i = 0; i < 4;++i) {
                for(int j = 0; j < 9; ++j) {
                    //Debug.Log(i+"|"+j+ "   :"+baseKeys[i, j]);

                    Vector3 currentPos = new Vector3(offsets[i] + (0.5f * cellDimensionX) + (j * cellDimensionX) + levelSize.xMin, 0.0f, ((3-i) * cellDimensionY) + (0.5f * cellDimensionY)+ levelSize.yMin);
                    GameObject current = Instantiate(cell, currentPos, cell.transform.rotation);
                    current.transform.localScale = new Vector3(cellDimensionX, cellDimensionY, 1.0f);
                    current.name = i + "|" + j;
                    current.transform.SetParent(transform);
                    m_cells.Add(current);
                    count++;
                }
            }
        }
        // else just find the tiles and fill the arraylist
        else
        {
            Transform[] cells = GetComponentsInChildren<Transform>();
            for (int i = 1; i < cells.Length; ++i)
            {
                m_cells.Add(cells[i].gameObject);
            }
            //print(m_cells.Count);
        }
        assignKeys(baseKeys);
	}

    public void assignKeys(string[,] keyMap) {
        GridCell cell;
        GameObject g;
        int count = 0;
        for(int i = 0; i < 4; ++i) {
            for(int j = 0; j < 9; ++j) {
                g = (GameObject) m_cells[count++];
                cell = g.GetComponent<GridCell>();
                cell.setTriggerKey(keyMap[i, j]);
            }
        }
    }

    void OnDrawGizmosSelected() {
        Gizmos.color = Color.green;

        
        Gizmos.DrawWireCube(new Vector3(transform.position.x + levelSize.center.x, 0.0f, transform.position.z + levelSize.center.y), new Vector3(levelSize.size.x, 0.0f, levelSize.size.y));
    }
	
}
