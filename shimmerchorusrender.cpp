/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

SHIMMER ECHO: A PITCH-MODULATED DELAY EFFECT W. CHORUS ON THE REPEAT SIGNAL
BY TOM "EFFECT MASTER" SOBOLIK

FOLLOW ME ON INSTA @TOMSOBOLIK

HOLLA!!!!!

*/

// -----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----
// library files && constants
#include <Bela.h>
#include <stdlib.h>
#include <Stk.h>
#include <Delay.h>
#include <PitShift.h>
#include <Mu45LFO.h>

// -----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----	-----
// pot assignments
int gDelayTimePot = 0;
int gFeedbackPot = 1;
int gDepthPot = 2;
int gRatePot = 3;

// globals
int gAudioFramesPerAnalogFrame;
int gPCount = 0;
double gPInterval = 0.5;
float gDelayMin = 60;
float gDelayMax = 1000;
float gRateMin = 0.1;
float gRateMax = 2;
float gMinDepth = 0;
float gMaxDepth = 7;
float smoothDelayTime, smoothDepth, smoothFreq, smoothFeedback;

// objects we need
stk::Delay p5Delay, p8Delay;
stk::PitShift p5Shifter, p8Shifter;
stk::DelayL chorusDelay;
Mu45LFO LFO;

int calcSampsFromMsec (BelaContext *context, float msec) {
	return (((context->audioSampleRate)) * msec) * 0.001;
}

bool setup(BelaContext *context, void *userData)
{
	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	p5Shifter.setShift(1.5);
	p8Shifter.setShift(2);
	float gDelayMaxSamps = calcSampsFromMsec(context, gDelayMax);
	p5Delay.setMaximumDelay(gDelayMaxSamps);
	p8Delay.setMaximumDelay(gDelayMaxSamps);
	chorusDelay.setMaximumDelay(gDelayMaxSamps);
	LFO = Mu45LFO();
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	// Declare any local variables you need
	float delayTime, feedback, depth, rate;
	
	float minDelay = gDelayMin, maxDelay = gDelayMax;
	float minDepth = gMinDepth, maxDepth = gMaxDepth;
	float minFreq = gRateMin, maxFreq = gRateMax;
	float p5next, p8next;
	float monoIn;
	float p5Delay_input, p8Delay_input;
	float p5Delay_output, p8Delay_output;
	float out_l, out_r;
	float LFOoutput;
        
    // Step through each frame in the audio buffer
    for(unsigned int n = 0; n < context->audioFrames; n++) 
    {
    	// --  --  --  --  --  --  --  --  --  --  --  --  --  --  -- 
		// Read the data from analog sensors, and calculate any alg params
		if(!(n % gAudioFramesPerAnalogFrame)) 
		{
			// READ, PROCESS, ASSIGN DATA FROM POT INPUT
			
			delayTime = analogRead(context, n/gAudioFramesPerAnalogFrame, gDelayTimePot);
			delayTime = map(delayTime, 0.0001, 0.827, minDelay, maxDelay);
			smoothDelayTime += 0.001*(delayTime - smoothDelayTime); 
			smoothDelayTime = (smoothDelayTime > maxDelay) ? maxDelay : smoothDelayTime;
			
			feedback = analogRead(context, n/gAudioFramesPerAnalogFrame, gFeedbackPot);
			feedback = map(feedback, 0.0001, 0.827, 0.1, 0.99);
			
			depth = analogRead(context, n/gAudioFramesPerAnalogFrame, gDepthPot);
			depth = map(depth, 0.0001, 0.827, minDepth, maxDepth);
			smoothDepth += 0.001*(depth - smoothDepth);
            smoothDepth = (smoothDepth > maxDepth) ? maxDepth : smoothDepth;
			
			rate = 0.5;
            smoothFreq += 0.001*(rate - smoothFreq);
            smoothFreq = (smoothFreq > maxFreq) ? maxFreq : smoothFreq;
            
            LFO.setFreq(rate, context->audioSampleRate);
            p5Delay.setDelay(calcSampsFromMsec(context, smoothDelayTime));
            p8Delay.setDelay(calcSampsFromMsec(context, smoothDelayTime));
		}
		
		monoIn = (audioRead(context, n, 0) + audioRead(context, n, 1)) / 2;
		
		LFOoutput = smoothDepth * LFO.tick();
		chorusDelay.setDelay(calcSampsFromMsec(context, (20+LFOoutput)));
		
		p5Delay_input = p5Shifter.tick(monoIn);
		p5next = p5Delay.nextOut();
		p5Delay_output = p5Delay.tick(p5Delay_input + feedback * p5next);
		
		p8Delay_input = monoIn;
		p8next = p8Delay.nextOut();
		p8next = p8Shifter.tick(p8next);
		p8Delay_output = p8Delay.tick(p8Delay_input + feedback * p8next);

		out_l = chorusDelay.tick((0.5*p5Delay_output)+(0.5*p8Delay_output));
		out_l += monoIn;
		out_r = out_l;
		
		audioWrite(context, n, 0, out_l);
		audioWrite(context, n, 1, out_r);
	}
}

void cleanup(BelaContext *context, void *userData)
{

}