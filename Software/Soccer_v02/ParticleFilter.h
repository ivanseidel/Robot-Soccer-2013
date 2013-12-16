#ifndef PARTICLEFILTER_H_
#define PARTICLEFILTER_H_


struct PFControl
{
	float motor1Speed;
	float motor2Speed;
};
typedef struct PFControl PFControl;

struct PFMeasurementDistance
{
	float angle;
	float distance;
};
typedef struct PFMeasurementDistance PFMeasurementDistance;

struct PFMeasurementLight
{
	float angle;
	float distance;
	int light;
};
typedef struct PFMeasurementLight PFMeasurementLight;

class ParticleFilter
{
	public:
		ParticleFilter();

		void prediction(PFControl control, float dt);
		void correctionDistance(PFMeasurementDistance measurementDistance);
		void correctionLight(PFMeasurementLight measurementLight);

	private:
		

};


#endif 