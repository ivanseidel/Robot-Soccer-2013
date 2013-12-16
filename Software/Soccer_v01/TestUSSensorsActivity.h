#include "views/TextView.h"

class TestUSSensorsActivity: public Activity
{
protected:
	TextView sampleText;
	
public:
	TestUSSensorsActivity(){
		Activity::Activity();

		ActivityName = String("Test US Sensors");
		
		sampleText = TextView("Sample Text");
		sampleText.o = Point(100, 120);
		addView(&sampleText);
	}

	void onResume(){
		
	}

	void onLoop(){
		
	};

	void onStop(){
		
	}
};
