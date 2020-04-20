// Conversion of coordinates
// -------------------------
// 3D Radon coordinates -> homogeneous coordinates:
//     planesRadonC -> planesHomC

__kernel void radonToHom( __global const float* planesRadonC,
                          __global float* planesHomC )
{
    const uint coordNb = get_global_id(0);

    planesRadonC += 3 * coordNb;
    float sinAzi = sin(planesRadonC[0]);
    float cosAzi = cos(planesRadonC[0]);
    float sinPol = sin(planesRadonC[1]);
    float cosPol = cos(planesRadonC[1]);

    planesHomC += 4 * coordNb;
    planesHomC[0] = sinPol * cosAzi;
    planesHomC[1] = sinPol * sinAzi;
    planesHomC[2] = cosPol;
    planesHomC[3] = -planesRadonC[2];
}
