#include "Material.h"

using namespace ci;

Material::Material()
: mDiffuseColor( vec3( 0.2f ) )
, mSpecularColor( vec3(0.0f) )
, mDiffuseReflectivity( 0.2f )
, mSpecularReflectivity( 0.2f )
, mEmisivity( 0.0f )
, mTransparency( 0.0f )
, mPad0( 0 ), mPad1( 0 )
{
}

Material& Material::diffuseColor( const vec3 c )
{
	mDiffuseColor = c;
	return *this;
}

Material& Material::specularColor( const vec3 c )
{
	mSpecularColor = c;
	return *this;
}

Material& Material::diffuseReflectivity( float v )
{
	mDiffuseReflectivity = v;
	return *this;
}

Material& Material::specularReflectivity( float v )
{
	mSpecularReflectivity = v;
	return *this;
}
Material& Material::emisivity( float v )
{
	mEmisivity = v;
	return *this;
}
Material& Material::transparency( float v )
{
	mTransparency = v;
	return *this;
}

const vec3 Material::getDiffuseColor() const
{
	return mDiffuseColor;
}

const vec3 Material::getSpecularColor() const
{
	return mSpecularColor;
}

float Material::getDiffuseReflectivity() const
{
	return mDiffuseReflectivity;
}

float Material::getSpecularReflectivity() const
{
	return mSpecularReflectivity;
}

float Material::getEmisivity() const
{
	return mEmisivity;
}
float Material::getTransparency() const
{
	return mTransparency;
}

void Material::setDiffuseColor( const vec3 c )
{
	mDiffuseColor = c;
}

void Material::setSpecularColor( const vec3 c )
{
	mSpecularColor = c;
}

void Material::setDiffuseReflectivity( float v )
{
	mDiffuseReflectivity = v;
}

void Material::setSpecularReflectivity( float v )
{
	mSpecularReflectivity = v;
}

void Material::setEmisivity( float v )
{
	mEmisivity = v;
}

void Material::setTransparency( float v )
{
	mTransparency = v;
}
