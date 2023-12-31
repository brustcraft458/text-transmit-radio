// Audio context
const AudioContext = window.AudioContext || window.webkitAudioContext
var audioCtx, gainNode, osciNode

// Init Component
const icon = document.querySelector("#icon-radio")
const form = document.querySelector("#text-form")
const formInput = form.querySelector("#text")
const formButton = form.querySelector("#button-submit")
const connectButton = form.querySelector("#button-connect")

// Button
formButton.addEventListener("click", () => {
    const binary = textToBinary(formInput.value)
    console.log(formInput.value)
    console.log("binary data: ", binary)
    
    // Play
    playback(binary)
})

connectButton.addEventListener("click", async() => {
    await playback("000000000000000000000000")
    await playback("111111111111111111111111")
    await playback("                        ")
})

// Play Audio
async function playback(binaryData) {
    const timeDelay = 350
    icon.classList.remove("border-secondary")

    for (let i = 0; i < binaryData.length; i++) {
        const bin = binaryData[i]
        
        // Play
        if (bin == '1') {
            icon.classList.add("border-primary")
            await playNote(500, 1.0, timeDelay)
            icon.classList.remove("border-primary")
        } else if (bin == '0') {
            icon.classList.add("border-warning")
            await playNote(500, 0.1, timeDelay)
            icon.classList.remove("border-warning")
        } else {
            await playNote(500, 0.035, timeDelay * 2)
        }

        // End
        await playNote(0, 0.0, timeDelay)
    }

    icon.classList.add("border-secondary")
}

// Note
async function playNote(note, volume, time) {
    if (!osciNode || !osciNode.context || osciNode.context.state != 'running') {
        // Create oscillator
        audioCtx = new AudioContext()
        osciNode = audioCtx.createOscillator()
        gainNode = audioCtx.createGain();

        osciNode.connect(gainNode);
        gainNode.connect(audioCtx.destination);
        osciNode.start()
    }

    osciNode.frequency.setValueAtTime(note, audioCtx.currentTime)
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