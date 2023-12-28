// Audio context
const AudioContext = window.AudioContext || window.webkitAudioContext
var audioCtx, osc

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
    const timeDelay = 150
    for (let i = 0; i < binaryData.length; i++) {
        const bin = binaryData[i]
        
        // Play
        if (bin == '1') {
            await playNote(1200, timeDelay)
        } else if (bin == '0') {
            await playNote(600, timeDelay)
        }

        // End
        await playNote(20, timeDelay)
    }
}

// Note
async function playNote(note, time) {
    if (!osc || !osc.context || osc.context.state != 'running') {
        // Create oscillator
        audioCtx = new AudioContext()
        
        osc = audioCtx.createOscillator()
        osc.frequency.setValueAtTime(0, audioCtx.currentTime)

        osc.connect(audioCtx.destination)
        osc.start()
    }

    osc.frequency.setValueAtTime(note, audioCtx.currentTime)
    await sleep(time)
    osc.frequency.setValueAtTime(10, audioCtx.currentTime)
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