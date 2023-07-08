const lightAddress = 'LOCAL_IP_ADDRESS/api/light';

// akes a GET request to light, returns string of state
const fetchState = async () => {
  const response = await fetch(lightAddress).catch(e => console.log(e.message));
  const responseBody = await response.json();
  if (!response.ok) {
    throw new Error('issue fetching state');
  }
  return responseBody['state'];
}

// makes a POST request to light, returns string of state
const postState = async (newState) => {
  const response = await fetch(lightAddress, {
    method: 'POST',
    headers: {
      'Accept' : 'application/json',
      'Content-Type' : 'application/json',
    },
    body: JSON.stringify({state: newState})
  }).catch(e => console.log(e.message));
  if (!response.ok) {
    throw new Error('issue changing state');
  }
  const responseBody = await response.json();
  return responseBody['state'];
}

// form handler
const submitState = () => {
  console.log('attempting to post');
  postState(document.getElementById('state-select').value)
    .then(newState => {
      changeState(newState);
    })
    .catch(e => {
      console.log(`Error: ${e.message}`);
    });
}

// updates webpage
const changeState = newState => {
  document.querySelector('#state').textContent = newState;
  document.querySelector('body').setAttribute('data-state', newState);
}

// loads current state on webpage load
fetchState()
  .then(newState => {
    changeState(newState);
  })
  .catch(e => {
    console.log(`Error: ${e.message}`);
  });