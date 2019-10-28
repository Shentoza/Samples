using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CatController :MonoBehaviour {

    private ArrayList m_pressedKeys;


    [SerializeField]
    private GameObject m_paw;

    [SerializeField]
    private Transform m_pawDestination;

    private float m_pawHeight;

    //[SerializeField]
    private float pawSpeed;

    [SerializeField]
    private AnimationCurve speedCurve;

    [SerializeField]
    private bool m_averageKeys;

    [SerializeField]
    private float m_timeToHit;
    private float m_hitProgress;

    [SerializeField]
    private Vector2 m_minMaxPawSpeed = new Vector2(10, 20);

    public bool IsLethal { get { return !m_bPlayedPoof; } }
    private bool m_bPlayedPoof = false;
    private ParticleSystem m_particles;

    public bool HittingBlocked {
        get;set;
    }

    void Awake()
    {
        m_particles = GetComponentInChildren<ParticleSystem>();

        pawSpeed = m_minMaxPawSpeed.y;
        HittingBlocked = false;
    }

    // Use this for initialization
    void Start() {
        m_pressedKeys = new ArrayList();
        KeyEvent.OnCellPressed += GridPressed;
        m_pawHeight = m_pawDestination.position.y;
    }

    void OnDisable() {
        KeyEvent.OnCellPressed -= GridPressed;
    }

    // Update is called once per frame
    void Update() {
        Vector3 newPos = Vector3.zero;
        
        if (m_pressedKeys.Count > 0) {
            foreach (GridCell cell in m_pressedKeys) {
                newPos += cell.transform.position;
            }
            GridCell lastCell = (GridCell)m_pressedKeys[m_pressedKeys.Count - 1];
            newPos = m_averageKeys ? newPos/m_pressedKeys.Count : lastCell.transform.position;
            newPos.y = m_pawHeight;
            m_pawDestination.position = newPos;

            float distance = Vector3.Distance(m_pawDestination.position, m_paw.transform.position);
            if (distance > (pawSpeed * Time.deltaTime)) {
                m_paw.transform.position += (m_pawDestination.position - m_paw.transform.position).normalized * pawSpeed * Time.deltaTime;
                
            }
            else {
                m_paw.transform.position = m_pawDestination.position;
                
            }
        }

        if (Input.GetKey(KeyCode.Space) && !HittingBlocked) {
            m_hitProgress = Mathf.Clamp01(m_hitProgress + (Time.deltaTime / m_timeToHit));
            pawSpeed = m_minMaxPawSpeed.x;
        }
        else {
            m_hitProgress = Mathf.Clamp01(m_hitProgress - (Time.deltaTime / m_timeToHit));
            pawSpeed = m_minMaxPawSpeed.y;
        }

        float height = m_pawHeight - (speedCurve.Evaluate(m_hitProgress) * m_pawHeight);
        m_paw.transform.position = new Vector3(m_paw.transform.position.x, height, m_paw.transform.position.z);

        if (height < 0.01f && !m_bPlayedPoof)
        {
            m_bPlayedPoof = true;
            m_particles.Play();
            SingleMusic.Instance.PlayEmptyPaw();
            //SingleMusic.Instance.PlayMouseSquish();
            //SingleMusic.Instance.PlayPickup();
        }
        else if (height > 2)
        {
            m_bPlayedPoof = false;
        }
    }

    private void GridPressed(GridCell cell, bool triggered) {
        if (!cell)
            return;

        if (triggered && !m_pressedKeys.Contains(cell)) {
            m_pressedKeys.Add(cell);
        }
        else {
            while (m_pressedKeys.Contains(cell))
                m_pressedKeys.Remove(cell);
        }
    }



}
