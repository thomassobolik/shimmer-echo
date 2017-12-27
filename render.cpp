#include <Bela.h>
#include <Mu45LFO.h>
#include <stdlib.h>
#include <DelayL.h>

// --- GLOBAL VARIABLES ---
int gAudioFramesPerAnalogFrame;
int gDepthPot = 0;
int gDelayTimePot = 1;
int gLFOFreqPot = 2;
float gDelayMin = 15;
float gDelayMax = 25;
float gLFOFreqMin_1 = 0.1;
float gLFOFreqMax_1 = 7;
float gMinDepth = 0;
float gMaxDepth = 15;
float smoothDelayTime, smoothDepth, smoothFreq, smoothFeedback;

// --- STK OBJECTS ---
stk::DelayL delay;
Mu45LFO LFO;

// --- FUNCTION 2 CALCULATE SAMPS FROM MSEC ---
float calcSampsFromMsec(BelaContext *context, float msec) {
	return (context->audioSampleRate * msec) * 0.001;
}

// --- SETUP: SET MAX DELAY TIME BASED ON SAMPLE RATE ---
bool setup(BelaContext *context, void *userData)
{
	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	int maxSamps = calcSampsFromMsec(context, gDelayMax);
	delay.setMaximumDelay(maxSamps);
	LFO = Mu45LFO();
	
	// Set the digital pins to input or output
	pinMode(context, 0, P8_08, INPUT);
	pinMode(context, 0, P8_07, INPUT);
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	float delayTime, depth, feedback, freq;
	float input, next, LFO_output, out_l, out_r;
	float minDelay, maxDelay, minFreq, maxFreq;
	
	minDelay = gDelayMin;
	maxDelay = gDelayMax;
	
	minFreq = gLFOFreqMin_1;
	maxFreq = gLFOFreqMax_1;
	
	float minDepth = gMinDepth;
	float maxDepth = gMaxDepth;
	
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		if(!(n % gAudioFramesPerAnalogFrame))
		{
			// --- READ, PROCESS, ASSIGN DATA FROM INPUT ---
			delayTime = analogRead(context, n/gAudioFramesPerAnalogFrame, 0);
			delayTime = map(delayTime, 0.0001, 0.827, minDelay, maxDelay);
			
			depth = analogRead(context, n/gAudioFramesPerAnalogFrame, 1);
			depth = map(depth, 0.0001, 0.827, minDepth, maxDepth);
			feedback = 0.065 * depth;
			
			freq = analogRead(context, n/gAudioFramesPerAnalogFrame, 2);
			freq = map(freq, 0.0001, 0.827, minFreq, maxFreq);
			
			smoothDelayTime += 0.001*(delayTime - smoothDelayTime);
			
            smoothDepth += 0.001*(depth - smoothDepth);
            smoothDepth = (smoothDepth > maxDepth) ? maxDepth : smoothDepth;
            smoothFreq += 0.001*(freq - smoothFreq);
            smoothFreq = (smoothFreq > maxFreq) ? maxFreq : smoothFreq;
            smoothDelayTime += 0.001*(delayTime - smoothDelayTime);
            smoothDelayTime = (smoothDelayTime > maxDelay) ? maxDelay : smoothDelayTime;
            
            LFO.setFreq(freq, context->audioSampleRate);
		}
		
		LFO_output = smoothDepth * LFO.tick();
		
		delay.setDelay(calcSampsFromMsec(context, smoothDelayTime + LFO_output));
		
		input = (audioRead(context, n, 0) + audioRead(context, n, 1)) / 2;
		
		next = delay.nextOut();
		
		delay.tick(input + next*feedback);
		
		out_l = input + next;
		out_r = out_l;
		
		audioWrite(context, n, 0, out_l);
		audioWrite(context, n, 1, out_r);
	}
}

void cleanup(BelaContext *context, void *userData)
{

}