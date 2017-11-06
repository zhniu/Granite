#version 450
precision mediump float;

#if HAVE_UV && defined(ALPHA_TEST)
layout(location = 1) in highp vec2 vUV;
#endif

#if (defined(HAVE_BASECOLORMAP) && HAVE_BASECOLORMAP) && defined(ALPHA_TEST)
layout(set = 2, binding = 0) uniform sampler2D uBaseColormap;
#endif

#ifdef ALPHA_TEST_ALPHA_TO_COVERAGE
layout(location = 0) out mediump vec4 FragColor;
#endif

void main()
{
#if defined(HAVE_BASECOLORMAP) && HAVE_BASECOLORMAP && defined(ALPHA_TEST)
    mediump vec4 base_color = texture(uBaseColormap, vUV);

    #if !defined(ALPHA_TEST_ALPHA_TO_COVERAGE)
        if (base_color.a < 0.5)
            discard;
    #endif

    #if defined(ALPHA_TEST_ALPHA_TO_COVERAGE)
        FragColor = base_color;
    #endif
#endif
}