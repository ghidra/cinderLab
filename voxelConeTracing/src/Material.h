#pragma once

#include "cinder/gl/gl.h"
using namespace glm;
class Material
{
public:
	Material();

	Material&			diffuseColor( const vec3 c );
	Material&			specularColor( const vec3 c );
	Material&			diffuseReflectivity( float v );
	Material&			specularReflectivity( float v );
	Material&			emisivity( float v );
	Material&			transparency( float v );

	const vec3	getDiffuseColor() const;
	const vec3	getSpecularColor() const;
	float				getDiffuseReflectivity() const;
	float				getSpecularReflectivity() const;
	float				getEmisivity() const;
	float				getTransparency() const;

	void				setDiffuseColor( const vec3 c );
	void				setSpecularColor( const vec3 c );
	void				setDiffuseReflectivity( float v );
	void				setSpecularReflectivity( float v );
	void				setEmisivity( float v );
	void				setTransparency( float v );
protected:
	vec3		mDiffuseColor;
	vec3			mSpecularColor;
	float		mDiffuseReflectivity;
	float		mSpecularReflectivity;
	float				mEmisivity;
	float				mTransparency;
	uint32_t			mPad0;
	uint32_t			mPad1;
};
