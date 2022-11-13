<!-- Vue 2.x  -->
<script>
  new Vue({
    el: '#example_vue',
    // Options...
      methods: {
        multiple: function (event) {
          this.hide()
          this.show()
        },
        hide:  function (event) {

        },
        show: function (event) {

        }
      },
      data() {
          return {
              // flag_name: true,
              // flag_value: false,
              flags: {},
              isHidden: false

          };
      },

      created() {
          fetch('arches_flags.json')
              .then(response => response.json())
              .then(data => (this.flags = data))
              .catch(err => console.log(err));
      }
  });
</script>

<div id="example_vue">
    <button type="button" v-on:click="isHidden = true" v-on:click="isHidden = !isHidden">Flag name</button>
<!--    <button type="button" v-on:click="multiple">Flag name</button>-->
    <button @click=true>Flag value</button>
    <table>
        <thead>
            <tr>
                <th v-if="!isHidden">Flag name</th>
                <th ref="th">Default value</th>
<!--                <th>Description</th>-->
<!--                <th>Description</th>-->
            </tr>
        </thead>
        <tbody>
            <tr v-for="flag in flags">
                <td v-if="!isHidden">{{ flag[0] }}</td>
                <td ref="td">{{ flag[1] }}</td>
<!--                <td>{{ flag[2] }}</td>-->
<!--                <td>{{ flag[3] }}</td>-->
<!--                <td v-if="flag_value">{{ flag[1] }}</td>-->
            </tr>
        </tbody>
    </table>
</div>


<!--<script src="https://cdnjs.cloudflare.com/ajax/libs/vue/2.5.17/vue.js"></script>-->

<!--<main id="vue-app">-->
<!--  <div ref="div">Hello world</div>-->
<!--  <button type="button" v-on:click="hideElements()">Hide!</button>-->
<!--  <button type="button" v-on:click="showElements()">Show!</button>-->
<!--</main>-->

<!--<script>-->
<!--    var app = new Vue({-->
<!--        el: "#vue-app",-->
<!--        methods: {-->

<!--            hideElements: function () {-->
<!--                this.$refs.div.style.display = "none";-->
<!--            },-->
<!--            showElements: function () {-->
<!--                this.$refs.div.style.display = "inherit";-->

<!--            }-->
<!--        }-->
<!--    });-->
<!--</script>-->

