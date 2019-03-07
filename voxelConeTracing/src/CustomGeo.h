///we are going to make meshes in the ssbo, no vbos here
namespace vct
{
    struct CustomGeo
    {
        CustomGeo() = default;
        CustomGeo( vec3 P_, vec3 N_, vec4 uv_, vec3 Cd_=vec3(0.5f), vec3 Cs_=vec3(1.0f), float Rd_=1.0f, float Rs_=1.0f, float e_=0.0f, float t_=0.0f )
        {
            P = P_;
            N = N_;
            uv = uv_;
            Cd = Cd_;
            Cs = Cs_;
            Rd = Rd_;
            Rs = Rs_;
            e = e_;
            t = t_;
            //lit=vec4(0.0f);
        }
        CustomGeo( vec3 P_, vec3 N_, vec2 uv_, vec3 Cd_=vec3(0.5f), vec3 Cs_=vec3(1.0f), float Rd_=1.0f, float Rs_=1.0f, float e_=0.0f, float t_=0.0f )
        {
            P = P_;
            N = N_;
            uv = vec4(uv_,0.0f,0.0f);
            Cd = Cd_;
            Cs = Cs_;
            Rd = Rd_;
            Rs = Rs_;
            e = e_;
            t = t_;
           //lit=vec4(0.0f);
        }
        alignas(16) vec3 P;//position
        alignas(16) vec3 N;//normal
        alignas(16) vec4 uv;//2 uv channels
        alignas(16) vec3 Cd;//color diffuse
        alignas(16) vec3 Cs;//color spec
        float Rd;//Reflectivity diffuse
        float Rs;//reflectivity specular
        float e;//emisivity
        float t;//tranparency
        //alignas(16) vec4 lit;//color spec
    };
    static const std::vector<CustomGeo> quadInstance
    {
        CustomGeo(vec3(-0.5f,-0.5f,0.0f),vec3(0.0f,1.0f,0.0f),vec2(0.0f,0.0f)),
        CustomGeo(vec3(0.5f,-0.5f,0.0f),vec3(0.0f,1.0f,0.0f),vec2(1.0f,0.0f)),
        CustomGeo(vec3(-0.5f,0.5f,0.0f),vec3(0.0f,1.0f,0.0f),vec2(0.0f,1.0f)),
        CustomGeo(vec3(-0.5f,0.5f,0.0f),vec3(0.0f,1.0f,0.0f),vec2(0.0f,1.0f)),
        CustomGeo(vec3(0.5f,-0.5f,0.0f),vec3(0.0f,1.0f,0.0f),vec2(1.0f,0.0f)),
        CustomGeo(vec3(0.5f,0.5f,0.0f),vec3(0.0f,1.0f,0.0f),vec2(1.0f,1.0f))
    };
    static const std::vector<CustomGeo> tetrahedron{
        CustomGeo(vec3(0.0f,1.0f,0.0),vec3(0.942808628082f,0.333334565163f,0.0),vec2(0.66666662693f,0.5f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(0.471405148506f,-0.333329707384f,-0.81649762392),vec3(0.942808628082f,0.333334565163f,0.0),vec2(0.999999940395f,0.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(0.471405148506f,-0.333329707384f,0.81649762392),vec3(0.942808628082f,0.333334565163f,0.0),vec2(0.999999940395f,1.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(0.0f,1.0f,0.0),vec3(-0.471404314041f,0.333334565163f,-0.816496312618),vec2(0.66666662693f,0.5f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(-0.942810297012f,-0.333329707384f,0.0),vec3(-0.471404314041f,0.333334565163f,-0.816496312618),vec2(0.0f,0.5f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(0.471405148506f,-0.333329707384f,-0.81649762392),vec3(-0.471404314041f,0.333334565163f,-0.816496312618),vec2(0.999999940395f,0.0f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(0.0f,1.0f,0.0),vec3(-0.471404314041f,0.333334565163f,0.816496312618),vec2(0.66666662693f,0.5f),vec3(0.0f,0.0f,1.0)),
        CustomGeo(vec3(0.471405148506f,-0.333329707384f,0.81649762392),vec3(-0.471404314041f,0.333334565163f,0.816496312618),vec2(0.999999940395f,1.0f),vec3(0.0f,0.0f,1.0)),
        CustomGeo(vec3(-0.942810297012f,-0.333329707384f,0.0),vec3(-0.471404314041f,0.333334565163f,0.816496312618),vec2(0.0f,0.5f),vec3(0.0f,0.0f,1.0)),
        CustomGeo(vec3(-0.942810297012f,-0.333329707384f,0.0),vec3(0.0f,-1.0f,0.0),vec2(0.0f,0.5f),vec3(1.0f,0.0f,1.0)),
        CustomGeo(vec3(0.471405148506f,-0.333329707384f,0.81649762392),vec3(0.0f,-1.0f,0.0),vec2(0.999999940395f,1.0f),vec3(1.0f,0.0f,1.0)),
        CustomGeo(vec3(0.471405148506f,-0.333329707384f,-0.81649762392),vec3(0.0f,-1.0f,0.0),vec2(0.999999940395f,0.0f),vec3(1.0f,0.0f,1.0))
    };
    static const std::vector<CustomGeo> cornell{
        CustomGeo(vec3(5.0f,-5.0f,-5.0),vec3(0.0f,0.0f,-1.0),vec2(1.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,5.0f,-5.0),vec3(0.0f,0.0f,-1.0),vec2(1.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,5.0f,-5.0),vec3(0.0f,0.0f,-1.0),vec2(0.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,-5.0f,5.0),vec3(1.0f,0.0f,0.0),vec2(1.0f,1.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(5.0f,5.0f,5.0),vec3(1.0f,0.0f,0.0),vec2(1.0f,1.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(5.0f,5.0f,-5.0),vec3(1.0f,0.0f,0.0),vec2(1.0f,0.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(-5.0f,-5.0f,5.0),vec3(0.0f,0.0f,1.0),vec2(0.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,5.0f,5.0),vec3(0.0f,0.0f,1.0),vec2(0.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,5.0f,5.0),vec3(0.0f,0.0f,1.0),vec2(1.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,-5.0f,-5.0),vec3(-1.0f,0.0f,0.0),vec2(0.0f,0.0f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(-5.0f,5.0f,-5.0),vec3(-1.0f,0.0f,0.0),vec2(0.0f,0.0f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(-5.0f,5.0f,5.0),vec3(-1.0f,0.0f,0.0),vec2(0.0f,1.0f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(5.0f,-5.0f,5.0),vec3(0.0f,-1.0f,0.0),vec2(1.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,-5.0f,-5.0),vec3(0.0f,-1.0f,0.0),vec2(1.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,-5.0f,-5.0),vec3(0.0f,-1.0f,0.0),vec2(0.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,5.0f,-5.0),vec3(0.0f,1.0f,0.0),vec2(1.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,5.0f,5.0),vec3(0.0f,1.0f,0.0),vec2(1.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,5.0f,5.0),vec3(0.0f,1.0f,0.0),vec2(0.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,5.0f,5.0),vec3(0.0f,1.0f,0.0),vec2(0.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,5.0f,-5.0),vec3(0.0f,1.0f,0.0),vec2(0.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,5.0f,-5.0),vec3(0.0f,1.0f,0.0),vec2(1.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,-5.0f,-5.0),vec3(0.0f,-1.0f,0.0),vec2(0.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,-5.0f,5.0),vec3(0.0f,-1.0f,0.0),vec2(0.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,-5.0f,5.0),vec3(0.0f,-1.0f,0.0),vec2(1.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,5.0f,5.0),vec3(-1.0f,0.0f,0.0),vec2(0.0f,1.0f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(-5.0f,-5.0f,5.0),vec3(-1.0f,0.0f,0.0),vec2(0.0f,1.0f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(-5.0f,-5.0f,-5.0),vec3(-1.0f,0.0f,0.0),vec2(0.0f,0.0f),vec3(0.0f,1.0f,0.0)),
        CustomGeo(vec3(5.0f,5.0f,5.0),vec3(0.0f,0.0f,1.0),vec2(1.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,-5.0f,5.0),vec3(0.0f,0.0f,1.0),vec2(1.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,-5.0f,5.0),vec3(0.0f,0.0f,1.0),vec2(0.0f,1.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,5.0f,-5.0),vec3(1.0f,0.0f,0.0),vec2(1.0f,0.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(5.0f,-5.0f,-5.0),vec3(1.0f,0.0f,0.0),vec2(1.0f,0.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(5.0f,-5.0f,5.0),vec3(1.0f,0.0f,0.0),vec2(1.0f,1.0f),vec3(1.0f,0.0f,0.0)),
        CustomGeo(vec3(-5.0f,5.0f,-5.0),vec3(0.0f,0.0f,-1.0),vec2(0.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(-5.0f,-5.0f,-5.0),vec3(0.0f,0.0f,-1.0),vec2(0.0f,0.0f),vec3(1.0f,1.0f,1.0)),
        CustomGeo(vec3(5.0f,-5.0f,-5.0),vec3(0.0f,0.0f,-1.0),vec2(1.0f,0.0f),vec3(1.0f,1.0f,1.0))
    };
}

