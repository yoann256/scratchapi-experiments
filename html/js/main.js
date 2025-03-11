// Declare some HTML elements
const pageBody = document.querySelector("body");
const pageHead = document.querySelector("head");

console.log("Main HTML elements successfully declared!");

// Declare other elements
const githubMain = document.querySelector(".github-main");
const githubOrg = document.querySelector(".github-org");
const reloadButton = document.querySelector(".reload-button");

console.log("Other HTML elements successfully declared!");

try {
    if (githubMain) {
        githubMain.addEventListener("click", function () {
            window.location.href = "https://github.com/yoann256/scratchapi-experiments";
        });
    } else {
        console.error("githubMain was not found or declared as null!");
    }

    if (githubOrg) {
        githubOrg.addEventListener("click", function () {
            window.location.href = "https://github.com/Scratch-Experiments-Group";
        });
    } else {
        console.error("githubOrg was not found or declared as null!");
    }
    
      if (reloadButton) {
        reloadButton.addEventListener("click", function () {
            window.location.reload();
        });
    } else {
        console.error("reloadButton was not found or declared as null!");
    }
} catch (error) { // FIX: Added 'error' parameter
    console.warn("Please run this script in a web browser environment! HTML/CSS/JS");
}
