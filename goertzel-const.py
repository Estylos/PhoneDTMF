import math

def generateCoef(sampleRate, blockSize, targetFreq):

    k = int( 0.5 + (blockSize * targetFreq) / sampleRate)
    w = (2 * math.pi / blockSize) * k
    coef = 2 * math.cos(w)
    
    return coef

def generateDtmfTonesCoef(sampleRate, blockSize):

    tones = [697, 770, 852, 941, 1209, 1336, 1477]
    for tone in tones:
        coef = generateCoef(sampleRate, blockSize, tone)
        print(str(tone) + " Hz, coef is "+ str(coef))


generateDtmfTonesCoef(9615, 100)