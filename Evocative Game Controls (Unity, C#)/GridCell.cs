using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GridCell :MonoBehaviour {

    [SerializeField]
    private Color lighted;
    [SerializeField]
    private Color basic;
    [SerializeField]
    private int bpm_forGlow = 117;
    [SerializeField]
    private Vector2 minMaxEmission = new Vector2(0.2f, 1f);
    [SerializeField]
    private AnimationCurve curve;
    [SerializeField]
    private float colorSpeed = 0.2f;
    [SerializeField]
    private float colorTurnBackSpeed = 10f;
    [SerializeField]
    private float afterglowDuration = 3f;

    private MeshRenderer m_meshR;
    public string triggerKey { get; private set; }
    public GameObject paw;
    float pawHeight;
    private float emissionFactor;
    private Color lastLightedColor;

    public void setTriggerKey(string s) {
        KeyEvent.Send(this, false);
        triggerKey = s;
    }

    // Use this for initialization
    void Start() {
        m_meshR = this.GetComponent<MeshRenderer>();
        m_meshR.material.color = basic;
        paw = GameObject.FindGameObjectWithTag("Paw");
        pawHeight = paw.transform.position.y;
        emissionFactor = minMaxEmission.x;
        lastLightedColor = basic;
    }

    // Update is called once per frame
    void Update() {
        CalculateLightedHue();
        CalculateCurrentEmissionFactor();
        HandleKeyInput();
    }

    void CalculateLightedHue()
    {
        float h, s, v;
        Color.RGBToHSV(lighted, out h, out s, out v);
        h += colorSpeed * Time.deltaTime;
        h = h % 360;
        lighted = Color.HSVToRGB(h, s, v);
    }

    private float lerpTimer = 0f;
    void CalculateCurrentEmissionFactor()
    {
        lerpTimer += Time.deltaTime;
        if (lerpTimer > afterglowDuration) lerpTimer = afterglowDuration;

        //timer += Time.deltaTime;
        emissionFactor = Mathf.PingPong(Time.time * bpm_forGlow / 60f, 1f);
        emissionFactor = curve.Evaluate(emissionFactor);
        emissionFactor = Mathf.Lerp(minMaxEmission.x, minMaxEmission.y, emissionFactor);

        // lerp color to the basic color
        lastLightedColor = Color.Lerp(lastLightedColor, basic, lerpTimer / afterglowDuration);

        // set the color
        m_meshR.material.SetColor("_EmissionColor", lastLightedColor * emissionFactor);
        RendererExtensions.UpdateGIMaterials(m_meshR);
    }

    void HandleKeyInput()
    {
        if (!string.IsNullOrEmpty(triggerKey))
        {
            if (Input.GetKeyDown(triggerKey))
            {
                lastLightedColor = lighted;
                KeyEvent.Send(this, true);
                lerpTimer = 0f;
            }
            else if (Input.GetKeyUp(triggerKey))
            {
                KeyEvent.Send(this, false);
            }
            // manage lighting
            if (Input.GetKey(triggerKey))
            {
                m_meshR.material.SetColor("_EmissionColor", lastLightedColor * emissionFactor);
                RendererExtensions.UpdateGIMaterials(m_meshR);
            }
        }
    }
}
