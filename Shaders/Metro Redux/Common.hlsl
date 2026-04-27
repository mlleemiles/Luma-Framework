#include "GameCBuffers.hlsl"
#include "../Includes/Color.hlsl"
#include "../Includes/Common.hlsl"
#include "../Includes/Oklab.hlsl"


struct ColorGradeConfig {
  float exposure;
  float highlights;
  float shadows;
  float contrast;
  float flare;
  float saturation;
  float dechroma;
  float hue_correction_strength;
  float3 hue_correction_source;
  float hue_correction_type; // 0 = input, 1 = output
  float blowout;
};

ColorGradeConfig DefaultColorGradeConfig() {
    ColorGradeConfig config;
    config.exposure = 1.f;
    config.contrast = 1.f;
    config.flare = 0.f;
    config.highlights = 1.f;
    config.shadows = 1.f;
    config.saturation = 1.f;
    config.dechroma = 0.f;
    config.hue_correction_strength = 0.f;
    config.hue_correction_source = 0;
    config.hue_correction_type = 0.f;
    config.blowout = 0.f;
    return config;
}

float Highlights(float x, float highlights, float mid_gray) {
    if (highlights == 1.f) return x;

    if (highlights > 1.f) {
        return max(x, lerp(x, mid_gray * pow(x / mid_gray, highlights), min(x, 5.f)));
    } else { // highlights < 1.f
        x /= mid_gray;
        return lerp(x, pow(x, highlights), step(1.f, x)) * mid_gray;
    }
}

float Shadows(float x, float shadows, float mid_gray) {
    if (shadows == 1.f) return x;

    const float ratio = max(safeDivision(x, mid_gray, 0), 0.f);
    const float base_term = x * mid_gray;
    const float base_scale = safeDivision(base_term, ratio, 0);

    if (shadows > 1.f) {
        float raised = x * (1.f + safeDivision(base_term, pow(ratio, shadows), 0));
        float reference = x * (1.f + base_scale);
        return max(x, x + (raised - reference));
    } else { // shadows < 1.f
        float lowered = x * (1.f - safeDivision(base_term, pow(ratio, 2.f - shadows), 0));
        float reference = x * (1.f - base_scale);
        return clamp(x + (lowered - reference), 0.f, x);
    }
}

float3 ApplyExposureContrastFlareHighlightsShadowsByLuminance(float3 untonemapped, float y, ColorGradeConfig config, float mid_gray = 0.18f) {
    if (config.exposure == 1.f && config.shadows == 1.f && config.highlights == 1.f && config.contrast == 1.f && config.flare == 0.f) {
        return untonemapped;
    }
    float3 color = untonemapped;

    color *= config.exposure;
    y *= config.exposure;

    // contrast & flare
    const float y_normalized = y / mid_gray;
    float flare = safeDivision(y_normalized + config.flare, y_normalized, 1);
    float exponent = config.contrast * flare;
    const float y_contrasted = pow(y_normalized, exponent) * mid_gray;

    // highlights
    float y_highlighted = Highlights(y_contrasted, config.highlights, mid_gray);

    // shadows
    float y_shadowed = Shadows(y_highlighted, config.shadows, mid_gray);

    const float y_final = y_shadowed;

    color = RestoreLuminance(color, y_final);

    return color;
}

float3 ApplySaturationBlowoutHueCorrectionHighlightSaturation(float3 tonemapped, float y, ColorGradeConfig config) {
    float3 color = tonemapped;
    if (config.saturation != 1.f || config.dechroma != 0.f || config.hue_correction_strength != 0.f || config.blowout != 0.f) {
        float3 perceptual_new = Oklab::linear_srgb_to_oklab(color);

        // if (config.hue_correction_strength != 0.f) {
        //     float3 perceptual_old = Oklab::linear_srgb_to_oklab(hue_reference_color);

        //     // Save chrominance to apply black
        //     float chrominance_pre_adjust = distance(perceptual_new.yz, 0);

        //     perceptual_new.yz = lerp(perceptual_new.yz, perceptual_old.yz, config.hue_correction_strength);

        //     float chrominance_post_adjust = distance(perceptual_new.yz, 0);

        //     // Apply back previous chrominance
        //     perceptual_new.yz *= safeDivision(chrominance_pre_adjust, chrominance_post_adjust, 1);
        // }

        if (config.dechroma != 0.f) {
            perceptual_new.yz *= lerp(1.f, 0.f, saturate(pow(y / (10000.f / 100.f), (1.f - config.dechroma))));
        }

        if (config.blowout != 0.f) {
            float percent_max = saturate(y * 100.f / 10000.f);
            // positive = 1 to 0, negative = 1 to 2
            float blowout_strength = 100.f;
            float blowout_change = pow(1.f - percent_max, blowout_strength * abs(config.blowout));
            if (config.blowout < 0) {
                blowout_change = (2.f - blowout_change);
            }

            perceptual_new.yz *= blowout_change;
        }

        perceptual_new.yz *= config.saturation;

        color = Oklab::oklab_to_linear_srgb(perceptual_new);

        // color = renodx::color::bt709::clamp::AP1(color);
    }
    return color;
}