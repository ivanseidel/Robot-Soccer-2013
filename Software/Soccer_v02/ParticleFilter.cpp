/*#include "ParticleFilter.h"

struct Particle
{
	float x;
	float y;
	float theta;
};

typedef struct Particle Particle;

#define NUM_PARTICLES 100;

static Particle particles[NUM_PARTICLES];
static float weights[NUM_PARTICLES];


static void initParticles()
{
	int i;
	for(i=0; i<NUM_PARTICLES; i++)
	{
		particles[i].x = 0;
		particles[i].y = 0;
		particles[i].theta = 0;		
	}	
}

static Particle predictParticle(Particle particle, MoveController control, int dt)
{
	// float dx = 
	// particle
}

static float measureDistanceWeight(Particle particle, PFMeasurementDistance measurementDistance)
{

}

static float measureLightWeight(Particle particle, PFMeasurementLight measurementLight)
{

}

static void resample()
{
	
}

ParticleFilter::ParticleFilter()
{
	initParticles();
}

void ParticleFilter::prediction(PFControl control, float dt)
{
	for(int i=0; i<numParticles; i++)
	{
		particles[i] = predictParticle(particles[i], control, dt);
	}
}

void ParticleFilter::correctionDistance(PFMeasurementDistance measurementDistance)
{
	for(int i=0; i<numParticles; i++)
	{
		weights[i] = measureDistanceWeight(particles[i], measurementDistance);
	}


}

void ParticleFilter::correctionLight(PFMeasurementLight measurementLight)
{
	for(int i=0; i<numParticles; i++)
	{
		weights[i] = measureLightWeight(particles[i], measurementLight);
	}
}*/