Shader "Custom/SSS"
{
    Properties
    {
        _Color ("Color", Color) = (1,1,1,1)
		_MainTex("Albedo (RGB)", 2D) = "white" {}
		_AO("AO Map", 2D) = "white" {}
		_Profile("Profile", 2D) = "white" {}
		_BumpMap("Bumpmap", 2D) = "bump" {}
		_SSSTint("_SSSTint", Color) = (1,1,1,1)
		_Scale("_Scale", Range(0,1)) = 0.5

        _Glossiness ("Smoothness", Range(0,1)) = 0.5
        _Metallic ("Metallic", Range(0,1)) = 0.0
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 200

        CGPROGRAM
        // Physically based Standard lighting model, and enable shadows on all light types
        #pragma surface surf StandardSSS fullforwardshadows

        // Use shader model 3.0 target, to get nicer looking lighting
        #pragma target 3.0
		#include "UnityPBSLighting.cginc"

        sampler2D _MainTex;
		sampler2D _BumpMap;
		sampler2D _AO;
		sampler2D _Profile;

        struct Input
        {
            float2 uv_MainTex;
			float2 uv_BumpMap;
			float3 worldPos;
			float3 worldNormal;
			INTERNAL_DATA
        };

        half _Glossiness;
        half _Metallic;
        fixed4 _Color;
		fixed4 _SSSTint;
		float _Scale;
		float thickness;

		void LightingStandardSSS_GI(SurfaceOutputStandard s, UnityGIInput data, inout UnityGI gi)
		{
			LightingStandard_GI(s, data, gi);
		}


		inline fixed4 LightingStandardSSS(SurfaceOutputStandard s, fixed3 viewDir, UnityGI gi)
		{
			// Original colour
			fixed4 pbr = LightingStandard(s, viewDir, gi);

			float3 L = gi.light.dir;
			float3 V = viewDir;
			float3 N = s.Normal;

			const float _Distortion = 0;
			float3 H = normalize(L + N * _Distortion);
			float VdotH = pow(saturate(dot(V, -H)), 1) * _Scale;
			float3 I = _SSSTint * saturate(VdotH + 0.4) * thickness;
			// Final add
			//pbr.rgb = pbr.rgb + gi.light.color * I;
			float3 profile = tex2D(_Profile, float2(dot(normalize(N), L) * 1.0 * 0.5 + 0.5, 0)).xyz;
			pbr.rgb = pbr.rgb + gi.light.color * profile * thickness * _Scale;
			//pbr.rgb = profile;
			//pbr.rgb = gi.light.color * I;
			return pbr;
		}

        // Add instancing support for this shader. You need to check 'Enable Instancing' on materials that use the shader.
        // See https://docs.unity3d.com/Manual/GPUInstancing.html for more information about instancing.
        // #pragma instancing_options assumeuniformscaling
        UNITY_INSTANCING_BUFFER_START(Props)
            // put more per-instance properties here
        UNITY_INSTANCING_BUFFER_END(Props)

        void surf (Input IN, inout SurfaceOutputStandard o)
        {
            // Albedo comes from a texture tinted by color
			fixed4 c = tex2D(_MainTex, IN.uv_MainTex) * _Color;
			fixed4 ao = tex2D(_AO, IN.uv_MainTex) * 0.5;
			//c = ao;
            o.Albedo = c.rgb;
			float3 worldNormal = WorldNormalVector(IN, o.Normal);
			//o.Albedo = length(fwidth(normalize(worldNormal))) / length(fwidth(IN.worldPos)) * 0.01;
            // Metallic and smoothness come from slider variables
			thickness = length(fwidth(normalize(worldNormal))) / length(fwidth(IN.worldPos)) * 0.01;
			thickness = 1 - ao.r;
			o.Normal = UnpackNormal(tex2D(_BumpMap, IN.uv_BumpMap));
			//o.Albedo.xyz = o.Normal.xyz;
			o.Metallic = _Metallic;
            o.Smoothness = _Glossiness;
            o.Alpha = c.a;
			o.Occlusion = ao.r * 2;
        }
        ENDCG
    }
    FallBack "Diffuse"
}
