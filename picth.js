// Audio context
const AudioContext = window.AudioContext || window.webkitAudioContext
var audioCtx, gainNode, osc

// Init Component
const form = document.querySelector("#text-form")
const formInput = form.querySelector("#text")
const formSubmit = form.querySelector("#button-submit")
formSubmit.addEventListener("click", () => {
    const binary = textToBinary(formInput.value)
    console.log(formInput.value)
    console.log("binary data: ", binary)
    
    // Play
    playback(binary)
})

// Play Audio
async function playback(binaryData) {
    const timeDelay = 800
    for (let i = 0; i < binaryData.length; i++) {
        const bin = binaryData[i]
        
        // Play
        if (bin == '1') {
            await playNote(1000, 1.0, timeDelay)
        } else if (bin == '0') {
            await playNote(1000, 0.1, timeDelay)
        }

        // End
        await playNote(0, 0.0, timeDelay)
    }
}

// Note
async function playNote(note, volume, time) {
    if (!osc || !osc.context || osc.context.state != 'running') {
        // Create oscillator
        audioCtx = new AudioContext()
        osc = audioCtx.createOscillator()
        gainNode = audioCtx.createGain();

        osc.connect(gainNode);
        gainNode.connect(audioCtx.destination);
        osc.start()
    }

    osc.frequency.setValueAtTime(note, audioCtx.currentTime)
    gainNode.gain.setValueAtTime(volume, audioCtx.currentTime);

    await sleep(time)
}

// Text to Binary
function textToBinary(text) {
    let binary = ''
    
    for (let i = 0; i < text.length; i++) {
        let binChar = text[i].charCodeAt(0).toString(2)
        // Limit 8 bit
        while (binChar.length < 8) {
            binChar = '0' + binChar
        }
        binary += binChar + ' '
    }
    
    return binary.trim()
} 

// Sleep with Promise
function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms))
}