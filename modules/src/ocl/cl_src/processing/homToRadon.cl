// Conversion of coordinates
// -------------------------
// (transformed) homogeneous coordinates -> 3D Radon coordinates:
//     H * initialPlane -> transformedCoords

__kernel void homToRadon( __constant float16* H,
                          __global const float4* initialPlanes,
                          __global float* transformedCoords )
{
    const uint coordNb = get_global_id(0);
    const float4 plane = initialPlanes[coordNb];
    const float4 transformedPlane = (float4)(dot((*H).s0123, plane),
                                             dot((*H).s4567, plane),
                                             dot((*H).s89ab, plane),
                                             dot((*H).scdef, plane));
    transformedCoords += 3 * coordNb;
    transformedCoords[0] = atan2(transformedPlane.y, transformedPlane.x);
    transformedCoords[1] = acos(transformedPlane.z);
    transformedCoords[2] = -transformedPlane.w;
}
