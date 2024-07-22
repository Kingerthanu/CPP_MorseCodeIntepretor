# CPP_MorseCodeIntepretor

  Project In Which Uses Audio Output On A System Endpoint/Playback To Listen For Morse Code Coming In And Interpreting It Into Its AlphaNumeric Translation. Also Uses User Input To Translate A AlphaNumeric Word Into Morse Code And Output The Noise In Morse Code At A Given Pitch For A Dynamic Length Of Time, Following The Morse-Code Standards For Time-Units. The Program Will Then Listen For This Morse-Code By The User Through Their Current Endpoint Even Through Loud Background Noises Being Emitted.

----------------------------------------------

<img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/88a0641b-10e0-4891-9a69-27d0c58fc038" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/88a0641b-10e0-4891-9a69-27d0c58fc038" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/88a0641b-10e0-4891-9a69-27d0c58fc038" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/88a0641b-10e0-4891-9a69-27d0c58fc038" alt="Cornstarch <3" width="65" height="49"> 


**The Breakdown:**

This Program Works With The Terminal And Users Input To Convert A Message Into Morse Code, Emit The Sound Of Their String And With WASAPI Detect The Morse And Reconvert It Into A Message Through Varying Volumes And Scales Of Background Noise.

The Program Deepdown Is Working On 3 Stages:

  - 1.) Translation To Of User Input Into Morse<br>
  - 2.) Emittion Of User Morse On PC<br>
  - 3.) Catching Of User Morse On PC<br>

The Program Starts By Asking The User For Their Given Alphanumeric String Input In Which They Want To Convert To Morse Code. After Hitting Enter, This Input Will Be Converted Into Morse Code Using **-** For A Long Beep & **.** For A Short Beep. The Program Works Off The International Standards For Morse Code With Spaces Between Words Being 6 Instead Of 7 Units. After The Input Is Converted, The Program Will Create A Detached Thread In Which Will Use A Audio Output Listening Algorithm In Order To Listen To The Computer For Incoming Morse Code Inputs. It Will Detect This Morse, And Using Lengths Of The Given Sounds To Interpolate What Type Of Character Is Being Communicated, Like A **' '**, **'-'**, or **'.'**. The Morse Code Will Be Outputted Using A waveOut(...) Algorithm In Which Will Emit A Sine Wave At A Base-Line Volume Of 0.8f Or 80%, Having +/- 0.1f (10%) In Amplitude. The Output Volume Is To Help Seperate From Background Noise In Which In This Build Can Be Very Loud As Their Baseline Lies At 0.0f And Has Difficulties Reaching Up To 0.7f, So Only Really System-Level Audio Will Interfere (Like Window's Notifications, Audio Chime When Changing Master Volume, etc.). After Receiving A Given Character--Denoted By A Single 3-Unit Wait--It Will Interpret The Given Morse Letter Back Into English. The Listening Thread Will Keep Listening For Morse Even After The Sounding Thread Has Concluded And Can Be Terminated With CTRL + C.

If You Know The Tempo Of A Current Morse Code Transaction You Can Run The Listening Thread Alone By Itself And Will Still Listen For The Passed Morse Asynchronously. The Listening Thread Also Works On 2 Stages Instead Of The Three Of Our Main Sounding Thread As We Only Have The 2 Stages Of:

  - 1.) Catching Of User Morse On PC<br>
  - 2.) Translation To Of User's Morse Input Into English<br>

The Listening Thread Will Pass Its Audio Buffer To A Window In Which Will Chart/Graph All Points To Show Us The Waveform Interpolation Of Our PC's Current Audio Ouput. Will Help Point Out Given Morse And Its Relative Range For Sensitivity As Certain Noises Like Musical Snares Or 808s Can Cause Unneccesary Blending.

Currently Works With Heavy Background Noise Where The Morse Code Is One Of The Primary Sounds. Currently We Are Using A Morse Seperative-Design Between Background Audio And Current Audio. But If Morse Code Is Being Emitted Inside Background Noise Or Through A YouTube Video It May Have Difficulties With This Current Build And May Want To Use Another Branch And Exclusively Have Just That More Dynamic Morse-Code Being Emitted On The Endpoint. This Is Because Unless It's Passing The Morse-Code As Raw Audio Data At A Baseline Of 0.8f (80% Volume) Usually It Will Have A Baseline Of 0.0f Causing Difficulties In Our Threshold Morse-Code Sine Waves Must Exceed To Be Listened to As Morse-Code And Through YouTube Videos Morse-Code Can Be Passed As Audio Data Much More Choppily As Sample Rates Can Range And Cause Some Fragmentation Or Corruption With Our System-Dynamic Codebase--Not Software-Dynamic Codebase.

Using A Pattern-Based Algorithm Was Attempted, But Many Difficulties Arise As Audio Kinda Sucks And Many Things Can Also Fall In Our Threshold Range If At 0.0f. Audio Can Also Compliment Eachother So If We Have Two Background Noises In Which Are Being Emitted At The Same Time At A Sample Value Of 0.2f and 0.3f It Could Merge This Sample In The Audio Buffer As 0.5f And If We Have A Threshold Of Detecting The Bottom-Most Position Of A Morse-Code Sine Wave At 0.5f We Could Have These Background Noises Being Considered As Morse-Code. This May Not Seem That Bad As We Do Use Millisecond Intervals Of Time To Interpret The Message Of The Audio Sample (I.E. 70ms Above Threshold Of 0.5f Is A **.**) As This Background Noise Could Only Maybe Peak Its Head Up For 2ms. But Lets Say We Are Listening To Morse And A Big Bass Sound Comes In From Some Background Noise At 0.3f For 200ms; Currently We Are Emitting The Sine Wave For **.** And Are On the Last 3ms Until The Sound Ends, If This Bass Now Comes In And Our Sine Wave Is Starting To Return To 0.0f (Sound Is Ending Going To Volume 0.0f [0%]) We Could Have Our Morse-Code Audio Sample Being 0.5f On The Edge Of Being Concluded And Considered As A **.**. But If This Bass Sound Comes In It Will Bump This Up To ~0.8f For These 200ms. This Sound Could Then Conclude And Based On Our Algorithm Would Then Now Be Considered As A **-** As Based On The Time Above The Threshold Of 0.5f We Never Got Below It Until This Background Bass. Many Other Issues Arose With Varying Designs But Other Versions Are Perfectly Workable Without Background Noise If The Variable Constants For Things Like The Wait Times For Dashes And Dots Are Known Beforehand.

<img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/739a779e-c28a-43d7-9d7c-9576a12beae8" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/739a779e-c28a-43d7-9d7c-9576a12beae8" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/739a779e-c28a-43d7-9d7c-9576a12beae8" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/739a779e-c28a-43d7-9d7c-9576a12beae8" alt="Cornstarch <3" width="65" height="49">

----------------------------------------------

<img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/ae006dac-e413-475a-8523-f48f0068144b" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/ae006dac-e413-475a-8523-f48f0068144b" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/ae006dac-e413-475a-8523-f48f0068144b" alt="Cornstarch <3" width="65" height="49"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/ae006dac-e413-475a-8523-f48f0068144b" alt="Cornstarch <3" width="65" height="49"> 


**Features:**

![2024-07-2119-33-35-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/83e9781b-2f19-4607-8ec0-0f83886a2c67)

![2024-07-2120-38-22-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/647d79dd-7f88-405a-bc76-610d226ca68b)


<img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/3cb63ad6-2fc7-4b4b-9fa8-142e78124d24" alt="Cornstarch <3" width="75" height="59"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/3cb63ad6-2fc7-4b4b-9fa8-142e78124d24" alt="Cornstarch <3" width="75" height="59"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/3cb63ad6-2fc7-4b4b-9fa8-142e78124d24" alt="Cornstarch <3" width="75" height="59"> <img src="https://github.com/Kingerthanu/CPP_MorseCodeIntepretor/assets/76754592/3cb63ad6-2fc7-4b4b-9fa8-142e78124d24" alt="Cornstarch <3" width="75" height="59">
