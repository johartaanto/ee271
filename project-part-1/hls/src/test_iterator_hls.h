#include "jitter_hls.h"
#include "sample_test_hls.h"

class TestIterator{
public:
    TestIterator(){}
    
    #pragma hls_design interface
    void CCS_BLOCK(run)(
        ac_channel<BoundingBoxHLS> &bbox_in, 
        ac_channel<TriangleHLS> &triangle_in,
        ac_channel<ConfigHLS> &config_in,
        ac_channel<SampleHLS> &sample_out   
    ){
        #ifndef __SYNTHESIS__
        while(triangle_in.available(1))
        #endif
        {
            BoundingBoxHLS bbox = bbox_in.read();
            TriangleHLS triangle = triangle_in.read();
            ConfigHLS config = config_in.read();

            // START CODE HERE
            // Create increment value from config.subsample
            // Iterate over box (using normal for loops)
                    // jitter sample
                    // test sample
                    // if hit, write out the sample (including RGB values)

            // Define subsample increment based on MSAA level
            SignedFixedPoint increment = 0;
            switch (config.subsample) {
                case 1: // MSAA 64x: sample is 1/8 pixel
                    increment.set_slc(0, (ac_int<8, false>)0x80); // 1/8 in fixed-point
                    increment.set_slc(8, (ac_int<16, false>)0x0000);
                    break;
                case 2: // MSAA 16x: sample is 1/4 pixel
                    increment.set_slc(0, (ac_int<9, false>)0x100); // 1/4 in fixed-point
                    increment.set_slc(9, (ac_int<15, false>)0x0000);
                    break;
                case 4: // MSAA 4x: sample is 1/2 pixel
                    increment.set_slc(0, (ac_int<10, false>)0x200); // 1/2 in fixed-point
                    increment.set_slc(10, (ac_int<14, false>)0x0000);
                    break;
                case 8: // MSAA 1x: no subsample
                    increment.set_slc(0, (ac_int<11, false>)0x400); // 1 in fixed-point
                    increment.set_slc(11, (ac_int<13, false>)0x0000);
                    break;
                default:
                    increment = 1; // Default to 1 if subsample is invalid
            }

            // Initialize the output sample and assign triangle colors
            SampleHLS sample;
            sample.R = triangle.R;
            sample.G = triangle.G;
            sample.B = triangle.B;

            // Iterate over the bounding box with the defined increment
            for (sample.x = bbox.lower_left.x; sample.x <= bbox.upper_right.x; sample.x += increment) {
                for (sample.y = bbox.lower_left.y; sample.y <= bbox.upper_right.y; sample.y += increment) {

                    // Jitter the sample
                    SampleHLS jitter = jitterSample.run(sample, config);

                    // Test the jittered sample against the triangle
                    bool isHit = sampleTest.run(triangle, jitter);

                    // If a hit is detected, write the sample to the output channel
                    if (isHit) {
                        sample_out.write(sample);
                    }
                }
            }


        // END CODE HERE
        }
    }
private:
    SampleTest sampleTest;
    JitterSample jitterSample;
};
