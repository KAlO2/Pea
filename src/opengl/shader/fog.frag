#version 330

// data members are with respect to Fog class
struct FogParameters
{
	vec4 color;
	float density;
	float start;
	float end;
	int mode;  // 0 = linear, 1 = exp, 2 = exp2
};

/**
 * @param z is the eye-coordinate distance between the viewpoint and the fragment center.
 * 		Values of z should be positive, representing eye-coordinate distance.
 */
float getFogFactor(FogParameters params, float z)
{
	float result;
	switch(params.mode)
	{
	case 0:
		result = (params.end - z) / (params.end - params.start);
		break;
	case 1:
		result = exp(-params.density * z);
		break;
	case 2:
		{
			float dxz = params.density * z;
			result = exp(-dxz * dxz);  // exp(-pow(params.density * z, 2.0))
		}
		break;
	default:
		result = 0.0f;
	}
		
	result = clamp(1.0 - result, 0.0, 1.0);
	return result;
}
