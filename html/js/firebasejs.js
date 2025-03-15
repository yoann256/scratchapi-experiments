  // Import the functions you need from the SDKs you need
  import { initializeApp } from "https://www.gstatic.com/firebasejs/11.4.0/firebase-app.js";
  import { getAnalytics } from "https://www.gstatic.com/firebasejs/11.4.0/firebase-analytics.js";
  import { getAuth, GoogleAuthProvider } from "https://www.gstatic.com/firebasejs/11.4.0/firebase-auth.js";
  // TODO: Add SDKs for Firebase products that you want to use
  // https://firebase.google.com/docs/web/setup#available-libraries

  // Your web app's Firebase configuration
  // For Firebase JS SDK v7.20.0 and later, measurementId is optional
  const firebaseConfig = {
    apiKey: "AIzaSyAmM0nyhiuVwAMhZMtsyvNKdc8Gem5bHEI",
    authDomain: "scratchapiexperiments.firebaseapp.com",
    projectId: "scratchapiexperiments",
    storageBucket: "scratchapiexperiments.firebasestorage.app",
    messagingSenderId: "539283707447",
    appId: "1:539283707447:web:404913214baf0d581e51b2",
    measurementId: "G-LNMKMG2LRW"
  };

  // Initialize Firebase
  const app = initializeApp(firebaseConfig);
  const analytics = getAnalytics(app);
  
  // Initialize Firebase Authentication and get a reference to the service 
  const auth = getAuth(app);
  
  const provider = new GoogleAuthProvider();
  
  // Some stuff
  provider.addScope('https://www.googleapis.com/auth/contacts.readonly');