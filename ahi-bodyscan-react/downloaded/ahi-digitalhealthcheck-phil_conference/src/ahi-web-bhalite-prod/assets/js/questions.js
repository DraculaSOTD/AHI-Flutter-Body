document.addEventListener("DOMContentLoaded", function () {
  const form = document.getElementById("questions-form");
  const dataIndexElements = Array.from(form.querySelectorAll("[data-index]"));
  const progressBar = document.querySelector("#progress-bar");
  const sectionControls = document.querySelector("#section-controls");

  let currentIndex = 0;

  const toggleElementVisibility = (element, isVisible) => {
    element.classList.toggle("hidden", !isVisible);
  };

  const updateVisibility = () => {
    toggleElementVisibility(dataIndexElements[currentIndex], true);
    toggleElementVisibility(sectionControls, currentIndex > 0);
    autoFocusFirstInput();
  };

  const autoFocusFirstInput = () => {
    const currentSection = dataIndexElements[currentIndex];
    const firstInput = currentSection.querySelector("input:not([type='date'])");
    if (firstInput) {
      firstInput.focus();
    }
  };

  const updateProgressBar = (isFull = false) => {
    const totalQuestions = dataIndexElements.length;
    const progressPercentage = isFull
      ? 100
      : (currentIndex / totalQuestions) * 100;
    progressBar.style.width = `${progressPercentage}%`;
  };

  const validateInputs = (event) => {
    const currentSection = dataIndexElements[currentIndex];
    const inputs = Array.from(currentSection.querySelectorAll("input"));

    for (const input of inputs) {
      if (!input.reportValidity()) {
        event.preventDefault();
        return false;
      }
    }
    return true;
  };

  const transitionElements = (oldIndex, newIndex) => {
    const oldElement = dataIndexElements[oldIndex];
    oldElement.classList.remove("fade-in");
    oldElement.classList.add("fade-out");
    toggleElementVisibility(oldElement, false);

    const newElement = dataIndexElements[newIndex];
    newElement.classList.remove("fade-out");
    // Force a reflow
    void newElement.offsetWidth;
    newElement.classList.add("fade-in");
    toggleElementVisibility(newElement, true);
  };

  const updateCSS = (direction) => {
    // Remove old style if it exists
    const oldStyle = document.getElementById("dynamic-css");
    if (oldStyle) oldStyle.remove();

    // Create new style
    const style = document.createElement("style");
    style.id = "dynamic-css";
    style.innerHTML = `
    @keyframes fadeIn {
      0% {
          opacity: 0;
          transform: translatey(${30 * direction}px);
      }
      100% {
          opacity: 1;
          transform: translatey(0);
      }
      }
      
      @keyframes fadeOut {
      0% {
          opacity: 1;
      }
      100% {
          opacity: 0;
      }
      }
      .fade-in {
          animation: fadeIn 750ms ease-in-out;
      }
      
      .fade-out{
          animation: fadeOut 250ms ease-in-out;
      }
    `;
    document.head.appendChild(style);
  };

  const incrementIndex = (event) => {
    if (validateInputs(event)) {
      if (event.target.checked || currentIndex < dataIndexElements.length - 1) {
        setTimeout(() => {
          updateCSS(-1);
          transitionElements(currentIndex, currentIndex + 1);
          currentIndex++;
          updateVisibility();
          updateProgressBar();
        }, 250);
      }
    }
  };

  const decrementIndex = () => {
    if (currentIndex > 0) {
      updateCSS(1);
      transitionElements(currentIndex, currentIndex - 1);
      currentIndex--;
      updateVisibility();
      updateProgressBar();
    }
  };

  const initializeVisibility = () => {
    dataIndexElements.forEach((element, index) => {
      toggleElementVisibility(element, index === currentIndex);
    });
  };

  const handleFormClick = (event) => {
    const target = event.target;
    const actions = {
      "button[data-next]": incrementIndex,
      "button[data-back]": decrementIndex,
      "input[type='radio']": incrementIndex,
    };

    for (const selector in actions) {
      if (target.matches(selector)) {
        actions[selector](event);
        break;
      }
    }
  };

  const handleFormSubmit = (event) => {
    if (currentIndex < dataIndexElements.length - 1) {
      event.preventDefault();
    } else {
      updateProgressBar(true);
    }
  };

  const handleKeyPress = (event) => {
    if (event.key === "Enter" && currentIndex < dataIndexElements.length - 1) {
      event.preventDefault();
      incrementIndex(event);
    }
  };

  initializeVisibility();
  form.addEventListener("click", handleFormClick);
  form.addEventListener("submit", handleFormSubmit);
  form.addEventListener("keypress", handleKeyPress);
});
